#include "command.h"
#include "hop.h"
#include "reveal.h"
#include "log.h"
#include "fcntl.h"
#include "color.h"
#include <stdio.h>
#include "iman.h"
#include "signal.h"
#include "seek.h"
#include "custom.h"
#include <stdlib.h>
#include "proclore.h"
#include <string.h>
#include <time.h>
#include "neonate.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include "linkedlist.h"
#include <ctype.h>

// Struct to hold alias and command pair
typedef struct Alias {
    char alias[50];
    char command[256];
} Alias;

#define MAX_ALIASES 100
Alias alias_table[MAX_ALIASES];
int alias_count = 0;

char *trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

void replace_tabs_with_spaces(char* str){
    for (int i = 0; str[i] != '\0'; i++){
        if (str[i]=='\t'){
            str[i]=' ';
        }
    }
}

void load_aliases(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror(RED "Error opening .myshrc" RESET);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *alias_key = strtok(line, "=");
        char *alias_value = strtok(NULL, "=");
        
        if (alias_key && alias_value) {
            alias_key = trim_whitespace(alias_key);
            alias_value = trim_whitespace(alias_value);
            alias_value[strcspn(alias_value, "\n")] = '\0';  // Remove newline
            
            if (alias_count < MAX_ALIASES) {
                strncpy(alias_table[alias_count].alias, alias_key, sizeof(alias_table[alias_count].alias) - 1);
                strncpy(alias_table[alias_count].command, alias_value, sizeof(alias_table[alias_count].command) - 1);
                alias_count++;
            } else {
                fprintf(stderr,RED "Alias limit reached. Cannot add more aliases.\n" RESET);
                break;
            }
        }
    }
    fclose(file);
}

#define MAX_COMMAND_LENGTH 1024

void replace_alias(char *command) {
    char temp[MAX_COMMAND_LENGTH];
    strcpy(temp, command);  // Copy the original command to a temp buffer

    for (int i = 0; i < alias_count; i++) {
        char *pos;
        // Search for the alias in the command string
        while ((pos = strstr(temp, alias_table[i].alias)) != NULL) {
            char new_command[MAX_COMMAND_LENGTH] = "";  // Buffer to store the modified command
            size_t alias_len = strlen(alias_table[i].alias);
            
            // Copy the part before the alias
            strncat(new_command, temp, pos - temp);
            
            // Add the command that corresponds to the alias
            strcat(new_command, alias_table[i].command);
            
            // Add the rest of the original string after the alias
            strcat(new_command, pos + alias_len);
            
            // Update the temp command with the new command (with alias replaced)
            strcpy(temp, new_command);
        }
    }

    // Copy the modified command back to the original command buffer
    strcpy(command, temp);
}

void handle_redirection(char* command, char *home_dir) {
    char* input_file = NULL;
    char* output_file = NULL;
    int append_mode = 0;

    char* in_redirect = strstr(command, "<");
    char* out_redirect = strstr(command, ">");
    
    if (in_redirect != NULL) {
        *in_redirect = '\0';
        input_file = strtok(in_redirect + 1, " \t\n");
        if (input_file != NULL && access(input_file, F_OK) == -1) {
            fprintf(stderr,RED "No such input file found!\n" RESET);
        }
    }

    if (out_redirect != NULL) {
        append_mode = (*(out_redirect + 1) == '>') ? 1 : 0;
        *out_redirect = '\0';
        output_file = strtok(out_redirect + 1 + append_mode, " \t\n");
    }

    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    if (input_file != NULL) {
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in < 0) {
            perror(RED "Error opening input file" RESET);
            return;
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    if (output_file != NULL) {
        int fd_out = open(output_file, O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC), 0644);
        if (fd_out < 0) {
            perror(RED "Error opening output file" RESET);
            return;
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }

    process_command(command, home_dir);

    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);

    close(saved_stdin);
    close(saved_stdout);
}

// Function to handle pipes
void handle_pipes(char* command, char* home_dir) {
    char* pipe_segments[10]; // Assuming a maximum of 10 pipes
    int num_pipes = 0;

    char *saveptr;
    char *token = strtok_r(command, "|", &saveptr);
    while (token != NULL && num_pipes < 10) {
        pipe_segments[num_pipes++] = strdup(token);
        token = strtok_r(NULL, "|", &saveptr);
    }

    // Check for invalid pipe usage
    for (int i = 0; i < num_pipes; i++) {
        if (strlen(pipe_segments[i]) == 0) {
            fprintf(stderr, RED "Invalid use of pipe\n" RESET);
        }
    }

    if (num_pipes > 1) {
        int pipefds[2 * (num_pipes - 1)];

        // Create pipes
        for (int i = 0; i < num_pipes - 1; i++) {
            if (pipe(pipefds + i * 2) < 0) {
                perror(RED "Pipe creation failed" RESET);
                exit(EXIT_FAILURE);
            }
        }

        for (int cmd_num = 0; cmd_num < num_pipes; cmd_num++) {
            pid_t pid = fork();
            if (pid == 0) {
                // Child process

                // If it's not the first command, get input from the previous pipe
                if (cmd_num != 0) {
                    dup2(pipefds[(cmd_num - 1) * 2], STDIN_FILENO);
                }

                // If it's not the last command, output to the next pipe
                if (cmd_num != num_pipes - 1) {
                    dup2(pipefds[cmd_num * 2 + 1], STDOUT_FILENO);
                }

                // Close all pipe file descriptors (important!)
                for (int i = 0; i < 2 * (num_pipes - 1); i++) {
                    close(pipefds[i]);
                }

                // Execute the command
                process_command(pipe_segments[cmd_num], home_dir);
                exit(0); // Ensure child exits after executing the command
            } else if (pid < 0) {
                perror(RED "fork" RESET);
                exit(EXIT_FAILURE);
            }
        }

        // Parent process: Close all pipes
        for (int i = 0; i < 2 * (num_pipes - 1); i++) {
            close(pipefds[i]);
        }

        // Wait for all child processes to finish
        for (int i = 0; i < num_pipes; i++) {
            wait(NULL);
        }
    }

    // Free allocated memory for pipe segments
    for (int i = 0; i < num_pipes; i++) {
        free(pipe_segments[i]);
    }
}

static void handle_log_command(const char *command, const char *home_dir) {
    // Check if the command contains "log"
    if (strstr(command, "log") != NULL) {
        return;
    }

    // Set the log directory based on the home directory
    set_log_directory(home_dir);

    // Check if the command is the same as the last one
    static char *last_command = NULL;  // Store the last command
    if (last_command != NULL && strcmp(last_command, command) == 0) {
        return;
    }

    // Update the last command
    if (last_command != NULL) {
        free(last_command);
    }
    last_command = strdup(command);

    // Trim the log file if necessary
    trim_log_file();

    // Log the command
    log_command(command);
}

extern void display_prompt(const char *home_dir); // Forward declaration of display_prompt function

double elapsed_time = 0;  // Global variable definition
char current_command[256];

static void background_process_handler(int sig) {
    int status;
    pid_t pid;
     while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        const char *command_name = get_process_name(pid);
        if (WIFEXITED(status)) {
            printf("Background process %d (%s) ended normally with exit status %d\n", pid, command_name, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Background process %d (%s) ended abnormally with signal %d\n", pid, command_name, WTERMSIG(status));
        }
        remove_process(pid);  // Clean up process list
    }
}

struct timeval start, end,end2;
void execute_command(const char *cmd, int background) {
    // Tokenize the command string into an array of arguments
char *args[256];
char *cmd_copy = strdup(cmd);
if (cmd_copy == NULL) {
    perror(RED "Error duplicating command" RESET);
    return;
}

int i = 0;
int in_quotes = 0;
char *arg_start = NULL;

for (char *token = cmd_copy; *token != '\0'; token++) {
    if (*token == '"' || *token == '\'') {  // Check for quotes
        in_quotes = !in_quotes;  // Toggle in_quotes flag
        if (in_quotes) {
            arg_start = token + 1;  // Start after the opening quote
        } else {
            *token = '\0';  // End the quoted argument
            args[i++] = strdup(arg_start);  // Save the quoted argument
            arg_start = NULL;  // Reset arg_start
        }
    } else if (*token == ' ' && !in_quotes) {  // Space outside of quotes
        if (arg_start != NULL) {  // If there's an ongoing argument
            *token = '\0';  // End the argument
            args[i++] = strdup(arg_start);  // Save the argument
            arg_start = NULL;  // Reset arg_start
        }
    } else if (arg_start == NULL) {
        arg_start = token;  // Start of a new argument
    }
}

// Add the last argument if there's any
if (arg_start != NULL && *arg_start != '\0') {
    args[i++] = strdup(arg_start);
}

args[i] = NULL;  // NULL-terminate the argument list
free(cmd_copy);

if (args[0] == NULL) {
    return;
}
    // Fork and execute the command

    pid_t pid = fork();
    // setpgid(pid, pid);  // Set child as its own group leader
    if (pid < 0) {
        perror(RED "fork failed" RESET);
        return;
    } else if (pid == 0) {  // Child process
    if(background){
        setpgid(pid,pid);
    }
        if (execvp(args[0], args) < 0) {
            printf(RED "ERROR : '%s' is not a valid command\n" RESET,cmd);
            exit(EXIT_FAILURE);
        }
    } else {  // Parent process
        if (background) {
            foreground_pid=-1;
              // Store the background process PID and command name
            add_process(pid, args[0]);
            // Print PID of the background process
            printf("Started background process PID: %d\n", pid);
            // Set up signal handler for background processes
            signal(SIGCHLD, background_process_handler);
        } else {
            // Set the global foreground PID
            foreground_pid = pid;
            strcpy(current_command,args[0]);
            // Wait for the foreground process to finish
            int status;
            if (waitpid(pid, &status, WUNTRACED) < 0) {
                perror(RED "waitpid failed" RESET);
            } else {
                gettimeofday(&end, NULL);
                elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
            }
            foreground_pid = -1; // Reset after the process finishes
        }
    }

    // Free dynamically allocated arguments
    for (int j = 0; j < i; j++) {
        free(args[j]);
    }
}


void process_command(const char *command, char *home_dir) {
    gettimeofday(&start, NULL);
     // Log the command before processing it
    handle_log_command(command, home_dir);
    
    // Make a modifiable copy of the command for strtok_r
    char *cmd_copy = strdup(command);
    if (cmd_copy == NULL) {
        perror(RED "Error duplicating command" RESET);
        // return;
    }
    replace_tabs_with_spaces(cmd_copy);
    // Replace alias if exists
    replace_alias(cmd_copy);

    // Split the input command by ';' to handle multiple commands
    char *cmd_segment;
    char *saveptr_segments;  // Save pointer for splitting by ';'
    cmd_segment = strtok_r(cmd_copy, ";", &saveptr_segments);

    while (cmd_segment != NULL) {
        // Trim leading/trailing whitespace
        while (isspace(*cmd_segment)) cmd_segment++;
        char *end = cmd_segment + strlen(cmd_segment) - 1;
        while (end > cmd_segment && isspace(*end)) end--;
        *(end + 1) = '\0';

        // Split the command segment by '&' to identify background execution
        char *background_cmd;
        char *saveptr_background;
    int counter=0;for(int i=0;i<strlen(cmd_segment);i++){if(cmd_segment[i]=='&')counter++;}
        background_cmd = strtok_r(cmd_segment, "&", &saveptr_background);
        // printf("%s\n",saveptr_background);

        while (background_cmd != NULL) {
            // Trim leading/trailing whitespace for each background command
            while (isspace(*background_cmd)) background_cmd++;
            end = background_cmd + strlen(background_cmd) - 1;
            while (end > background_cmd && isspace(*end)) end--;
            *(end + 1) = '\0';

            if (strlen(background_cmd) > 0) {
                int background = counter--;

            // Check if the command contains pipes
            if (strchr(background_cmd, '|') != NULL) {
                // Command contains pipes
                handle_pipes(background_cmd, home_dir);
            } 
            // Check if the command contains redirection operators
            else if (strchr(background_cmd, '<') != NULL || strchr(background_cmd, '>') != NULL) {
                // Command contains redirection operators
                handle_redirection(background_cmd, home_dir);
            } 
                //seding to custom command here
                // Check for "log execute" command and handle it
            else if(execute_custom_function(background_cmd,home_dir)==1){}
            else if (strncmp(background_cmd, "log execute", 11) == 0) {
                int index = atoi(background_cmd + 12);  // Extract index from command
                if (index > 0) {
                    char *cmd_to_execute = get_command_from_log(index);
                    if (cmd_to_execute != NULL) {
                        // printf("Executing command from log: %s\n", cmd_to_execute);
                        // Recursively call process_command with the retrieved command
                        process_command(cmd_to_execute, home_dir);
                        free(cmd_to_execute);  // Free the command retrieved from log
                    } else {
                        printf(RED "Invalid command index.\n" RESET);
                    }
                } else {
                    printf(RED "Invalid index.\n" RESET);
                }
            }

                // Handle specific commands
                else if (strncmp(background_cmd, "hop", 3) == 0) {
                    if (strlen(background_cmd) == 3) {
                        hop_command(home_dir, "~");  // No argument, go to home directory
                    } else {
                        char *args[256];
                        int i = 0;
                        char *saveptr_hop_args;  // Save pointer for splitting hop command arguments
                        char *token = strtok_r(background_cmd, " ", &saveptr_hop_args);
                        while (token != NULL && i < sizeof(args)/sizeof(args[0]) - 1) {
                            args[i++] = token;
                            token = strtok_r(NULL, " ", &saveptr_hop_args);
                        }
                        args[i] = NULL;  // NULL-terminate the argument list

                        for (int j = 1; j < i; j++) {
                            hop_command(home_dir, args[j]);
                        }
                    }
                } else if (strncmp(background_cmd, "reveal", 6) == 0) {
                    char *args[256];
                    int i = 0;
                    char *saveptr_reveal_args;  // Save pointer for splitting reveal command arguments
                    char *token = strtok_r(background_cmd, " ", &saveptr_reveal_args);
                    while (token != NULL && i < sizeof(args)/sizeof(args[0]) - 1) {
                        args[i++] = token;
                        token = strtok_r(NULL, " ", &saveptr_reveal_args);
                    }
                    args[i] = NULL;  // NULL-terminate the argument list

                    char *flags = NULL;
                    char *path = NULL;

                    // Collect flags and path
                    for (int j = 1; j < i; j++) {
                        if (args[j][0] == '-') {
                            if (flags == NULL) {
                                flags = strdup(args[j]);
                            } else {
                                char *temp = malloc(strlen(flags) + strlen(args[j]) + 1);
                                if (temp == NULL) {
                                    perror(RED "Error allocating memory" RESET);
                                    free(cmd_copy);
                                }
                                strcpy(temp, flags);
                                strcat(temp, args[j] + 1);  // Skip the leading '-'
                                free(flags);
                                flags = temp;
                            }
                        } else {
                            path = args[j];
                            break;
                        }
                    }

                    if (path == NULL) {
                        path = ".";
                    }

                    reveal_command(flags, path, home_dir);

                    if (flags != NULL) {
                        free(flags);
                    }
                } else if (strncmp(background_cmd, "exit", 4) == 0) {
                    // printf("Exiting shell...\n");
                    exit(EXIT_SUCCESS);  // Exit the program
                }
                else if (strncmp(background_cmd, "neonate -n", 10) == 0) {
                    int time_arg = atoi(background_cmd + 11);  // Extract time argument
                    if (time_arg > 0) {
                        neonate(time_arg);  // Call the neonate function
                    } else {
                        printf(RED "Invalid time argument.\n" RESET);
                    }
                }  else if (strncmp(background_cmd, "log purge", 9) == 0) {
                    log_purge();  // Clear the log file
                    // printf("Log file purged.\n");
                }
                else if (strncmp(command, "proclore", 8) == 0) {
                    const char *pid_str = command + 9;  // Extract PID string
                    proclore(pid_str);
                }
                

                else if (strncmp(background_cmd, "activities", 10) == 0) {
                    sort_process_list();
                    ProcessNode *current = get_process_list_head();                    
                    while (current) {
                        const char *state = get_process_state(current->pid);
                        printf("[%d] : %s - %s\n", current->pid, current->command, state);
                        current = current->next;
                    }
                }
                else if (strcmp(background_cmd, "log") == 0 ) {
                    print_log();  // Print the log file content
                }// Check for the "ping" command and handle it
                else if(strncmp(background_cmd,"bg",2)==0){
                    int index = atoi(background_cmd + 3);  // Extract index from command
                 // Check if the process exists
                    if (kill(index, 0) == -1) {
                        if (errno == ESRCH) {
                            fprintf(stderr,RED "No such process found\n" RESET);
                        } else {
                            perror(RED "Error sending signal" RESET);
                        }
                    }
                    else
                    send_signal(index,18);
                }
                else if(strncmp(background_cmd,"fg",2)==0){
                    int index = atoi(background_cmd + 3);  // Extract index from command
                 // Check if the process exists
                    if (kill(index, 0) == -1) {
                        if (errno == ESRCH) {
                            fprintf(stderr,RED "No such process found\n" RESET);
                        } else {
                            perror(RED "Error sending signal" RESET);
                        }
                    }
                    // Send SIGCONT to the process to resume it if it's stopped
                    if (kill(index, SIGCONT) == -1) {
                        perror(RED "Error sending SIGCONT to process" RESET);
                    }
                    // Set the global foreground PID
                    foreground_pid = index;
                    const char* name=get_process_name(index);
                    strcpy(current_command,name);
                    // Wait for the foreground process to finish
                    int status;
                    if (waitpid(index, &status, WUNTRACED) < 0) {
                        perror(RED "waitpid failed" RESET);
                    } else {
                        gettimeofday(&end2, NULL);
                        elapsed_time = (end2.tv_sec - start.tv_sec) + (end2.tv_usec - start.tv_usec) / 1000000.0;
                    }
                    remove_process(foreground_pid);
                    foreground_pid = -1; // Reset after the process finishes
                }else if (strncmp(background_cmd, "iMan", 4) == 0) {
                    char *cmd_name = strtok(background_cmd + 5, " ");  // Extract the command name after 'iMan '
                    if (cmd_name) {
                        fetch_man_page(cmd_name);
                    } else {
                        printf(RED "Usage: iMan <command_name>\n" RESET);
                    }
                }
                else if (strncmp(background_cmd, "ping", 4) == 0) {
                    char *args[256];
                    int i = 0;
                    char *saveptr_ping_args;  // Save pointer for splitting ping command arguments
                    char *token = strtok_r(background_cmd, " ", &saveptr_ping_args);
                    while (token != NULL && i < sizeof(args) / sizeof(args[0]) - 1) {
                        args[i++] = token;
                        token = strtok_r(NULL, " ", &saveptr_ping_args);
                    }
                    args[i] = NULL;  // NULL-terminate the argument list

                    if (i == 3) {  // Expecting exactly 3 arguments: "ping", "<pid>", "<signal_number>"
                        pid_t pid = atoi(args[1]);
                        int signal_number = atoi(args[2]);
                        send_signal(pid, signal_number);
                    } else {
                        printf(RED "Usage: ping <pid> <signal_number>\n" RESET);
                    }
                }else if (strncmp(background_cmd, "seek", 4) == 0) {
                    char *args[256];
                    int i = 0;
                    char *saveptr_seek_args;  // Save pointer for splitting seek command arguments
                    char *token = strtok_r(background_cmd, " ", &saveptr_seek_args);
                    while (token != NULL && i < sizeof(args)/sizeof(args[0]) - 1) {
                        args[i++] = token;
                        token = strtok_r(NULL, " ", &saveptr_seek_args);
                    }
                    args[i] = NULL;  // NULL-terminate the argument list

                    if (i > 1) {
                        // Handle seek command
                        seek_command_handler(args, i,home_dir);
                    } else {
                        printf(RED "Usage: seek <flags> <search> <target_directory>\n" RESET);
                    }
                } else {
                    // Execute other commands
                    execute_command(background_cmd, background);
                }
            }

            background_cmd = strtok_r(NULL, "&", &saveptr_background);
        }

        cmd_segment = strtok_r(NULL, ";", &saveptr_segments);
    }

    free(cmd_copy);
}

