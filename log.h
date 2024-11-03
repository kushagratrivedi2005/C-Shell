#ifndef LOG_H
#define LOG_H

#include <sys/types.h>

// Function to set the log directory based on the home directory
void set_log_directory(const char *home_dir);
#define MAX_LOG_ENTRIES 14  // Define the maximum number of log entries
#define LOG_FILE "command.log"  // Log file name, without directory

// Function declarations
void init_log();
void cleanup_log();
void trim_log_file();
void log_command(const char *command);
void print_log();
void log_purge();
char* get_command_from_log(int index);

#endif // LOG_H
