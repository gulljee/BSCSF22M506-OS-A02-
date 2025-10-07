/*
* Programming Assignment 02: lsv1.4.0
* This version supports:
*   - Default multi-column directory listing (down then across)
*   - Long listing format using -l
*   - Horizontal (row-major) listing using -x
*   - Alphabetical sorting of filenames
* Usage:
*       $ ./lsv1.4.0
*       $ ./lsv1.4.0 -l
*       $ ./lsv1.4.0 -x
*       $ ./lsv1.4.0 /home /etc
*       $ ./lsv1.4.0 -l /home/kali
*       $ ./lsv1.4.0 -x /home/kali
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <strings.h>

extern int errno;

// ------------------------- DATA STRUCTURES -------------------------

typedef struct {
    char **names;   // array of filenames
    int count;      // number of files
    int max_len;    // length of longest filename
} FileList;

// ------------------------- FUNCTION DECLARATIONS -------------------------

void do_ls(const char *dir, int horizontal);
void do_ls_long(const char *dir);
void print_permissions(mode_t mode);

FileList read_filenames(const char *dir);
void free_filelist(FileList *fl);
int get_terminal_width();
void print_files_column(FileList fl);
void print_files_horizontal(FileList fl);
int compare_filenames(const void *a, const void *b); // for qsort

// ------------------------- MAIN FUNCTION -------------------------

int main(int argc, char *argv[]) {
    int opt;
    int long_listing = 0;
    int horizontal = 0;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l':
                long_listing = 1;
                break;
            case 'x':
                horizontal = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [directory...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // If no directories are given, use current directory
    if (optind == argc) {
        if (long_listing)
            do_ls_long(".");
        else
            do_ls(".", horizontal);
    } else {
        for (int i = optind; i < argc; i++) {
            printf("Directory listing of %s:\n", argv[i]);
            if (long_listing)
                do_ls_long(argv[i]);
            else
                do_ls(argv[i], horizontal);
            puts("");
        }
    }

    return 0;
}

// ------------------------- SIMPLE COLUMN LISTING -------------------------

void do_ls(const char *dir, int horizontal) {
    FileList fl = read_filenames(dir);
    if (fl.count > 0) {
        if (horizontal)
            print_files_horizontal(fl);
        else
            print_files_column(fl); // default "down then across"
        free_filelist(&fl);
    }
}

// ------------------------- LONG LISTING -------------------------

void do_ls_long(const char *dir) {
    FileList fl = read_filenames(dir); // read & sort
    if (fl.count == 0) return;

    for (int i = 0; i < fl.count; i++) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, fl.names[i]);

        struct stat fileStat;
        if (stat(path, &fileStat) == -1) {
            perror("stat");
            continue;
        }

        print_permissions(fileStat.st_mode);
        printf("%2lu ", fileStat.st_nlink);

        struct passwd *pw = getpwuid(fileStat.st_uid);
        struct group  *gr = getgrgid(fileStat.st_gid);
        printf("%s %s ", pw ? pw->pw_name : "unknown", gr ? gr->gr_name : "unknown");

        printf("%8ld ", fileStat.st_size);

        char *timeStr = ctime(&fileStat.st_mtime);
        timeStr[strlen(timeStr) - 1] = '\0'; // remove newline
        printf("%s ", timeStr);

        printf("%s\n", fl.names[i]);
    }

    free_filelist(&fl);
}


// ------------------------- PERMISSION STRING -------------------------

void print_permissions(mode_t mode) {
    char perms[11];
    perms[0] = S_ISDIR(mode) ? 'd' :
               S_ISLNK(mode) ? 'l' :
               S_ISCHR(mode) ? 'c' :
               S_ISBLK(mode) ? 'b' :
               S_ISFIFO(mode) ? 'p' :
               S_ISSOCK(mode) ? 's' : '-';

    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';

    printf("%s ", perms);
}

// ------------------------- DYNAMIC FILENAME ARRAY WITH SORTING -------------------------

int compare_filenames(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcasecmp(fa, fb);
}

FileList read_filenames(const char *dir) {
    DIR *dp = opendir(dir);
    struct dirent *entry;
    FileList fl = {NULL, 0, 0};

    if (!dp) {
        perror("Cannot open directory");
        return fl;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        fl.names = realloc(fl.names, sizeof(char*) * (fl.count + 1));
        fl.names[fl.count] = strdup(entry->d_name);
        int len = strlen(entry->d_name);
        if (len > fl.max_len) fl.max_len = len;
        fl.count++;
    }

    closedir(dp);

    // Sort filenames alphabetically
    if (fl.count > 1) {
        qsort(fl.names, fl.count, sizeof(char *), compare_filenames);
    }

    return fl;
}

void free_filelist(FileList *fl) {
    for (int i = 0; i < fl->count; i++)
        free(fl->names[i]);
    free(fl->names);
    fl->names = NULL;
    fl->count = 0;
    fl->max_len = 0;
}

// ------------------------- TERMINAL COLUMN PRINTING -------------------------

int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 80; // fallback width
    return w.ws_col;
}

// "Down then across" default column printing
void print_files_column(FileList fl) {
    int term_width = get_terminal_width();
    int spacing = 2;
    int col_width = fl.max_len + spacing;
    int cols = term_width / col_width;
    if (cols == 0) cols = 1;

    int rows = (fl.count + cols - 1) / cols; // ceil division

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = r + c * rows;
            if (idx >= fl.count)
                continue;
            printf("%-*s", col_width, fl.names[idx]);
        }
        printf("\n");
    }
}

// "Row-major" horizontal printing for -x option
void print_files_horizontal(FileList fl) {
    int term_width = get_terminal_width();
    int spacing = 2;
    int col_width = fl.max_len + spacing;
    int pos = 0; // current horizontal position

    for (int i = 0; i < fl.count; i++) {
        if (pos + col_width > term_width) {
            printf("\n");
            pos = 0;
        }
        printf("%-*s", col_width, fl.names[i]);
        pos += col_width;
    }
    printf("\n");
}
