# Feature 2
### What is the crucial difference between the stat() and lstat() system calls? In the context of the ls command, when is it more appropriate to use lstat()?
- The crucial difference between stat() and lstat() is that stat() follows symbolic links and returns information about the target file, whereas lstat() returns information about the link itself. In the context of the ls command, lstat() is more appropriate because it allows the program to correctly identify and display symbolic links rather than mistakenly showing the details of their target files.
### The st_mode field in struct stat is an integer that contains both the file type (e.g., regular file, directory) and the permission bits. Explain how you can use bitwise operators (like &) and predefined macros (like S_IFDIR or S_IRUSR) to extract this information.
- The st_mode field in struct stat is an integer that encodes both the file type and permission bits. Bitwise operators and predefined macros are used to extract this information—for example, (st_mode & S_IFMT) == S_IFDIR checks if the file is a directory, while (st_mode & S_IRUSR) tests whether the user has read permission.

# Feature 3
### Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop through the list of filenames insufficient for this task?
- In the "down then across" column format, filenames are printed vertically in columns before moving to the next column, so the output fills the screen top-to-bottom and then left-to-right. A simple single loop is not enough because it prints items in a single row or column only; instead, nested loops and index calculations are needed to position filenames correctly across rows and columns.
### What is the purpose of the ioctl system call in this context? What would be the limitations of your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting the terminal size?
- The ioctl system call is used to get the current terminal width, allowing the program to adjust the number of columns dynamically based on the screen size. Without it, using a fixed width like 80 columns could cause poor alignment—either too many columns on wide terminals or wrapped, messy output on narrow ones.

# Feature 4
### Compare the implementation complexity of the "down then across" (vertical) printing logic versus the "across" (horizontal) printing logic. Which one requires more pre-calculation and why?
- The down then across (vertical) printing logic is more complex because it requires calculating the number of rows and columns that fit in the terminal and correctly mapping each filename’s position in the grid. This means more pre-calculation is needed compared to the simpler horizontal (“across”) logic, which just prints items one by one in order.
### Describe the strategy you used in your code to manage the different display modes (-l, -x, and default). How did your program decide which function to call for printing?
- In my code, I handled different display modes using simple condition checks for the command-line options. If the -l flag was found, the program called the long listing function; if -x was found, it called the horizontal printing function; otherwise, it defaulted to the vertical (down-then-across) printing function.

# Feature 5
### Why is it necessary to read all directory entries into memory before you can sort them? What are the potential drawbacks of this approach for directories containing millions of files?
- It’s necessary to read all directory entries into memory before sorting because qsort() (and similar sorting functions) need direct access to all items in an array to compare and reorder them. The drawback is that for directories with millions of files, this can use a huge amount of memory and slow down performance, or even cause the program to run out of memory.
### Explain the purpose and signature of the comparison function required by qsort(). How does it work, and why must it take const void * arguments?
- The comparison function in qsort() defines how two elements are compared during sorting. Its signature is int compare(const void *a, const void *b). It takes const void * arguments so it can work with any data type — the function must cast them to the correct type inside. It returns a negative value if a < b, zero if they’re equal, and positive if a > b.

# Feature 6
### How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.
- ANSI escape codes work by sending special character sequences to the terminal that control text formatting, color, and style. The terminal interprets these sequences instead of printing them as text. For example, the code \033[32m changes the text color to green, and \033[0m resets it back to normal.
### To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others.
- To check if a file is executable, you examine the permission bits in the st_mode field from the stat structure. Specifically, you check S_IXUSR for the owner, S_IXGRP for the group, and S_IXOTH for others. If any of these bits are set, it means the file is executable by that user category.

