#include "reveal.h"
#include "color.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <time.h>  // Include this for time-related functions

#define PATH_MAX 4096
static char previous_dir[PATH_MAX] = "";

void print_file_info(const struct stat *file_stat, const char *name) {
    // Print file type and permissions
    printf((S_ISDIR(file_stat->st_mode)) ? "d" : "-");
    printf((file_stat->st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat->st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat->st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat->st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat->st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat->st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat->st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat->st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat->st_mode & S_IXOTH) ? "x " : "- ");

    // Print number of hard links
    printf("%lu ", file_stat->st_nlink);

    // Print owner and group
    struct passwd *pw = getpwuid(file_stat->st_uid);
    struct group *gr = getgrgid(file_stat->st_gid);
    if (pw == NULL || gr == NULL) {
        print_error("Error getting file owner or group");
        return;
    }
    printf("%s %s ", pw->pw_name, gr->gr_name);

    // Print file size
    printf("%ld ", file_stat->st_size);

    // Print last modification time
    char time_buff[80];
    struct tm *tm_info = localtime(&(file_stat->st_mtime));
    if (tm_info == NULL) {
        print_error("Error converting modification time");
        return;
    }
    strftime(time_buff, sizeof(time_buff), "%b %d %H:%M", tm_info);
    printf("%s ", time_buff);

    // Print file name with color coding
    if (S_ISDIR(file_stat->st_mode)) {
        printf("%s%s%s\n", DIR_COLOR, name, RESET);  // Blue for directories
    } else if (file_stat->st_mode & S_IXUSR) {
        printf("%s%s%s\n", EXEC_COLOR, name, RESET);  // Green for executables
    } else {
        printf("%s%s%s\n", FILE_COLOR, name, RESET);  // White for regular files
    }
}

int compare_entries(const void *a, const void *b) {
    const struct dirent **entry_a = (const struct dirent **)a;
    const struct dirent **entry_b = (const struct dirent **)b;
    return strcmp((*entry_a)->d_name, (*entry_b)->d_name);
}

void reveal_command(const char *flags, const char *path, const char *home_dir) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;

    char new_dir[PATH_MAX];
    bool show_hidden = false;
    bool show_long = false;

    // Handle special cases for paths (same as before)
    if (path == NULL || strcmp(path, ".") == 0) {
        if (getcwd(new_dir, sizeof(new_dir)) == NULL) {
            print_error("Error getting current directory");
            return;
        }
    } else if (strcmp(path, "~") == 0) {
        if (home_dir == NULL) {
            print_error("Home directory is not set");
            return;
        }
        strcpy(new_dir, home_dir);
    } else if (strcmp(path, "-") == 0) {
        if (getcwd(new_dir, sizeof(new_dir)) == NULL) {
            print_error("Error getting current directory");
            return;
        }
    } else if (strcmp(path, "..") == 0) {
        if (getcwd(new_dir, sizeof(new_dir)) != NULL) {
            char *last_slash = strrchr(new_dir, '/');
            if (last_slash != NULL && last_slash != new_dir) {
                *last_slash = '\0';
            }
        } else {
            print_error("Error getting current directory");
            return;
        }
    } else if (path[0] == '~') {
        snprintf(new_dir, sizeof(new_dir), "%s%s", home_dir, path + 1);
    } else {
        strncpy(new_dir, path, sizeof(new_dir));
        new_dir[sizeof(new_dir) - 1] = '\0';
    }

    // Check if the path is a file or directory
    if (stat(new_dir, &file_stat) == -1) {
        print_error("Error getting file status");
        return;
    }

    // Process flags
    if (flags != NULL) {
        for (size_t i = 0; i < strlen(flags); i++) {
            char flag = flags[i];
            if (flag == 'a') {
                show_hidden = true;
            } else if (flag == 'l') {
                show_long = true;
            }
        }
    }

    if (S_ISREG(file_stat.st_mode)) {
        // It's a regular file, print info directly
        if (show_long) {
            print_file_info(&file_stat, path);
        } else {
            printf("%s%s%s\n", FILE_COLOR, path, RESET);
        }
        return;  // No need to proceed further
    }

    // Save the current directory as the previous directory
    char current_dir[PATH_MAX];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        print_error("Error getting current directory");
        return;
    }
    strncpy(previous_dir, current_dir, sizeof(previous_dir));
    previous_dir[sizeof(previous_dir) - 1] = '\0';

    // If it's not a regular file, proceed with directory handling
    if ((dir = opendir(new_dir)) == NULL) {
        print_error("Error opening directory");
        return;
    }

    struct dirent *entries[1024];
    int count = 0;

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }

        if (count < sizeof(entries) / sizeof(entries[0])) {
            entries[count++] = entry;
        } else {
            print_error("Too many entries");
            break;
        }
    }

    qsort(entries, count, sizeof(entries[0]), compare_entries);

    for (int i = 0; i < count; ++i) {
        char full_path[PATH_MAX];
        // Ensure the combined length fits in full_path
        if (snprintf(full_path, sizeof(full_path), "%s/%s", new_dir, entries[i]->d_name) >= sizeof(full_path)) {
            print_error("Path length exceeds buffer size");
            continue;
        }
        
        if (stat(full_path, &file_stat) == -1) {
            print_error("Error getting file status");
            continue;
        }

        if (show_long) {
            print_file_info(&file_stat, entries[i]->d_name);
        } else {
            if (S_ISDIR(file_stat.st_mode)) {
                printf("%s%s%s\n", DIR_COLOR, entries[i]->d_name, RESET);
            } else if (file_stat.st_mode & S_IXUSR) {
                printf("%s%s%s\n", EXEC_COLOR, entries[i]->d_name, RESET);
            } else {
                printf("%s%s%s\n", FILE_COLOR, entries[i]->d_name, RESET);
            }
        }
    }

    closedir(dir);
}
