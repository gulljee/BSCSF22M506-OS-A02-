/*
* Programming Assignment 02: lsv1.0.0
* This version supports:
*   - Basic directory listing
*   - Long listing format using -l
* Usage:
*       $ ./lsv1.0.0
*       $ ./lsv1.0.0 -l
*       $ ./lsv1.0.0 /home /etc
*       $ ./lsv1.0.0 -l /home/kali
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


extern int errno;

// Function declarations
void do_ls(const char *dir);
void do_ls_long(const char *dir);
void print_permissions(mode_t mode);

// ------------------------- MAIN FUNCTION -------------------------

int main(int argc, char *argv[]) {
    int opt;
    int long_listing = 0;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l':
                long_listing = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directory...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // If no directory is given, use current directory
    if (optind == argc) {
        if (long_listing)
            do_ls_long(".");
        else
            do_ls(".");
    } 
    else {
        for (int i = optind; i < argc; i++) {
            printf("Directory listing of %s:\n", argv[i]);
            if (long_listing)
                do_ls_long(argv[i]);
            else
                do_ls(argv[i]);
            puts("");
        }
    }

    return 0;
}

// ------------------------- SIMPLE LISTING -------------------------

void do_ls(const char *dir) {
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;
        printf("%s\n", entry->d_name);
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
}

// ------------------------- LONG LISTING -------------------------

void do_ls_long(const char *dir) {
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (!dp) {
        perror("Cannot open directory");
        return;
    }

    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        struct stat fileStat;
        if (stat(path, &fileStat) == -1) {
            perror("stat");
            continue;
        }

        // Print file permissions
        print_permissions(fileStat.st_mode);

        // Number of links
        printf("%2lu ", fileStat.st_nlink);

        // Owner and group
        struct passwd *pw = getpwuid(fileStat.st_uid);
        struct group  *gr = getgrgid(fileStat.st_gid);
        printf("%s %s ", 
            pw ? pw->pw_name : "unknown",
            gr ? gr->gr_name : "unknown");

        // File size
        printf("%8ld ", fileStat.st_size);

        // Modification time
        char *timeStr = ctime(&fileStat.st_mtime);
        timeStr[strlen(timeStr) - 1] = '\0';  // remove newline
        printf("%s ", timeStr);

        // File name
        printf("%s\n", entry->d_name);
    }

    closedir(dp);
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
