#include "display.h"
#include "hop.h"
#include "reveal.h"
#include "color.h"
#include "log.h"
#include "signal.h"
#include "custom.h"
#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

int main() {

    char home_dir[MAX_PATH_LENGTH];
    if (getcwd(home_dir, sizeof(home_dir)) == NULL) {
        perror(RED "Error getting home directory" RESET);
        return EXIT_FAILURE;
    }

    // Load aliases from .myshrc
    load_aliases(".myshrc");
    load_functions(".myshrc");
    
    // Initialize log tracking with home directory
    set_log_directory(home_dir);
    init_log();
    setup_signal_handlers();

    while (1) {
        display_prompt(home_dir);

        char command[256];
        if (fgets(command, sizeof(command), stdin) == NULL) {
            // Check if fgets returns NULL due to EOF (Ctrl-D)
            if (feof(stdin)) {
                handle_sigquit(SIGQUIT);  // Handle Ctrl-D by logging out
            } else {
                perror(RED "Error reading input" RESET);
                exit(EXIT_FAILURE);
            }
        }
        // Remove the newline character from input
        command[strcspn(command, "\n")] = '\0';

        // Process the command
        process_command(command, home_dir);
    }

    cleanup_log();  // Clean up memory used for logging
    return EXIT_SUCCESS;
}
