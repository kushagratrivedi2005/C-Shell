#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "color.h"
#include "command.h"
#include "custom.h"

#define MAX_FUNCTION_NAME 256
#define MAX_FUNCTION_BODY 1024

typedef struct Function {
    char name[MAX_FUNCTION_NAME];
    char body[MAX_FUNCTION_BODY];
    struct Function *next;
} Function;

static Function *function_list = NULL;

// Function to add a new function to the list
static void add_function(const char *name, const char *body) {
    Function *new_func = malloc(sizeof(Function));
    if (new_func == NULL) {
        perror(RED "Error allocating memory for function" RESET);
        exit(EXIT_FAILURE);
    }

    strncpy(new_func->name, name, MAX_FUNCTION_NAME - 1);
    new_func->name[MAX_FUNCTION_NAME - 1] = '\0';
    strncpy(new_func->body, body, MAX_FUNCTION_BODY - 1);
    new_func->body[MAX_FUNCTION_BODY - 1] = '\0';
    new_func->next = function_list;
    function_list = new_func;
}

// Load functions from the .myshrc file
void load_functions(const char *myshrc_file) {
    // printf("%d\n", 1111);  // Debug statement
    FILE *file = fopen(myshrc_file, "r");
    if (file == NULL) {
        perror(RED "Error opening .myshrc file" RESET);
        return;
    }

    char line[1024];
    char function_name[MAX_FUNCTION_NAME];
    char function_body[MAX_FUNCTION_BODY];
    int in_function = 0;

    while (fgets(line, sizeof(line), file)) {
        // Strip comments (ignore anything after #)
        char *comment_pos = strchr(line, '#');
        if (comment_pos != NULL) {
            *comment_pos = '\0';  // Null-terminate to ignore the comment
        }

        // Trim leading/trailing whitespace
        char *trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;  // Trim leading spaces
        char *end = trimmed_line + strlen(trimmed_line) - 1;
        while (end > trimmed_line && isspace(*end)) end--;  // Trim trailing spaces
        *(end + 1) = '\0';  // Null-terminate after trimming

        // Check for function start
        if (strncmp(trimmed_line, "func ", 5) == 0) {
            in_function = 1;
            sscanf(trimmed_line + 5, "%s", function_name);  // Get the function name

            // Remove parentheses if they exist
            char *paren_pos = strchr(function_name, '(');
            if (paren_pos) {
                *paren_pos = '\0';  // Null-terminate to remove the parentheses
            }

            function_body[0] = '\0';  // Reset function body
        } 
        else if (in_function && strstr(trimmed_line, "{") != NULL) {
            // Ignore the opening curly brace '{'
            continue;
        }
        else if (in_function && strncmp(trimmed_line, "}", 1) == 0) {
            // Function ends at closing curly brace
            in_function = 0;
            add_function(function_name, function_body);
        } 
        else if (in_function && strlen(trimmed_line) > 0) {
            // Append lines to function body if it's not empty
            strncat(function_body, trimmed_line, sizeof(function_body) - strlen(function_body) - 1);
            strncat(function_body, "\n", sizeof(function_body) - strlen(function_body) - 1);  // Add newline
        }
    }

    fclose(file);
}



// Execute a custom function if it matches
int execute_custom_function(const char *command, char *home) {
    char function_name[MAX_FUNCTION_NAME];
    char function_args[MAX_FUNCTION_BODY] = {0};
    char *arg_start;

    // Extract function name
    sscanf(command, "%s", function_name);

    // Extract arguments if present
    arg_start = strchr(command, ' ');
    if (arg_start) {
        arg_start++;  // Move past the space
        strcpy(function_args, arg_start);
    }

    // Search for the function in the function list
    Function *func = function_list;
    while (func != NULL) {
        if (strcmp(func->name, function_name) == 0) {
            // Duplicate function body
            char *body_copy = strdup(func->body);
            if (!body_copy) {
                perror(RED "Error duplicating function body" RESET);
                return 0;
            }

            // Replace both $1 and "$1" with function_args (without adding quotes)
            char *new_body = malloc(MAX_FUNCTION_BODY);
            if (!new_body) {
                perror(RED "Error allocating memory for function body" RESET);
                free(body_copy);
                return 0;
            }
            new_body[0] = '\0';  // Start with an empty string

            char *current_pos = body_copy;
            while (*current_pos) {
                // Check for $1
                char *arg_pos = strstr(current_pos, "$1");
                char *quoted_arg_pos = strstr(current_pos, "\"$1\"");
                
                if (arg_pos && (!quoted_arg_pos || arg_pos < quoted_arg_pos)) {
                    // Handle $1 without quotes
                    strncat(new_body, current_pos, arg_pos - current_pos);  // Copy part before $1
                    strcat(new_body, function_args);  // Insert function_args
                    current_pos = arg_pos + 2;  // Move past $1
                } else if (quoted_arg_pos) {
                    // Handle "$1"
                    strncat(new_body, current_pos, quoted_arg_pos - current_pos);  // Copy part before "$1"
                    strcat(new_body, function_args);  // Insert function_args
                    current_pos = quoted_arg_pos + 4;  // Move past "$1"
                } else {
                    // No more $1 or "$1", copy the rest
                    strcat(new_body, current_pos);
                    break;
                }
            }

            // Execute commands in the modified function body
            char *cmd = strtok(new_body, "\n");  // Split by newlines
            while (cmd != NULL) {
                // Trim whitespace from the command
                while (isspace(*cmd)) cmd++;
                char *end = cmd + strlen(cmd) - 1;
                while (end > cmd && isspace(*end)) end--;
                *(end + 1) = '\0';

                if (strlen(cmd) > 0) {
                    // printf("Executing: %s\n", cmd);  // Debug: print the command without quotes
                    process_command(cmd, home);  // Execute the command
                }
                cmd = strtok(NULL, "\n");  // Process next command
            }

            free(body_copy);
            free(new_body);
            return 1;  // Exit after executing the matched function
        }
        func = func->next;
    }
    return 0;
}
