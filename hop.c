#include "hop.h"
#include <stdio.h>
#include <stdlib.h>
#include "color.h"
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

static char previous_dir[MAX_PATH_LENGTH] = "";

void hop_command(char *home_dir, char *path) {
    char new_dir[MAX_PATH_LENGTH];
    char resolved_path[MAX_PATH_LENGTH];

    // Handle special cases for paths
    if (strcmp(path, "~") == 0) {
        strcpy(new_dir, home_dir);  // Change to home directory
    } else if (strcmp(path, "-") == 0) {
        if (previous_dir[0] != '\0') {
            strcpy(new_dir, previous_dir);  // Change to previous directory
        } else {
            fprintf(stderr,RED "No previous directory to switch to.\n" RESET);
            return;
        }
    } else if (strcmp(path, "..") == 0 || strcmp(path, ".") == 0) {
        // For ".." and ".", simply use the current directory and combine
        if (getcwd(new_dir, sizeof(new_dir)) != NULL) {
            if (strcmp(path, "..") == 0) {
                // Go one directory up
                char *last_slash = strrchr(new_dir, '/');
                if (last_slash != NULL) {
                    *last_slash = '\0';  // Remove the last part of the path
                }
            }
        } else {
            perror(RED "Error getting current directory" RESET);
            return;
        }
    } else if (path[0] == '~') {
        // Change to directory relative to home
        snprintf(new_dir, sizeof(new_dir), "%s%s", home_dir, path + 1);
    } else {
        // Treat as an absolute or relative path
        strcpy(new_dir, path);
    }

    // Resolve the full path
    if (realpath(new_dir, resolved_path) == NULL) {
        perror(RED "Error resolving path" RESET);
        return;
    }

    // Save the current directory as the previous directory
    if (getcwd(previous_dir, sizeof(previous_dir)) == NULL) {
        perror(RED "Error getting current directory" RESET);
        return;
    }

    // Attempt to change directory
    if (chdir(resolved_path) != 0) {
        perror(RED "Error changing directory" RESET);
    } else {
        // Print the absolute path of the new working directory
        printf("%s\n", resolved_path);
    }
}
