#include "display.h"
#include "color.h"
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

extern double elapsed_time;        // Use external variable to track elapsed time
extern char current_command[256];  // Buffer to store the current command

void display_prompt(const char *home_dir) {
    // Get the username
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        print_error("Error getting username");
        exit(EXIT_FAILURE);
    }
    char *username = pw->pw_name;

    // Get the system name
    char system_name[HOST_NAME_MAX];
    if (gethostname(system_name, sizeof(system_name)) != 0) {
        print_error("Error getting system name");
        exit(EXIT_FAILURE);
    }

    // Get the current working directory
    char current_dir[PATH_MAX];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        print_error("Error getting current directory");
        exit(EXIT_FAILURE);
    }

    // Determine the relative path or use the full path
    char relative_path[PATH_MAX];
    if (strncmp(current_dir, home_dir, strlen(home_dir)) == 0) {
        // Current directory is within the home directory
        if (strlen(current_dir) == strlen(home_dir)) {
            // Current directory is exactly the home directory
            snprintf(relative_path, sizeof(relative_path), "~");
        } else {
            // Current directory is a subdirectory of the home directory
            snprintf(relative_path, sizeof(relative_path), "~%s", current_dir + strlen(home_dir));
        }
    } else {
        // Current directory is outside the home directory
        snprintf(relative_path, sizeof(relative_path), "%s", current_dir);
    }

    // Print the prompt with color coding
    printf("%s<%s@%s:%s>%s", PROMPT_COLOR, username, system_name, relative_path, RESET);

    // Check for and print any messages about foreground process time
    if (elapsed_time > 2) {
       printf("%s :", current_command);             // Print the command normally
    printf("%s%.0fs%s", PROMPT_COLOR, elapsed_time, RESET); // Print the elapsed time in color
    printf(">");                                 // Close the prompt
    elapsed_time = 0;                            // Reset elapsed time after printing
    } else {
        printf(">"); // Close the prompt
    }

    fflush(stdout); // Ensure prompt is displayed immediately
}
