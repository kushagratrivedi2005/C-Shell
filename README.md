# How to Run

1. **Compile the project**:
   - Run the following command to compile all `.c` files:
     ```bash
     make
     ```

2. **Execute the program**:
   - After successful compilation, run the program with:
     ```bash
     ./a.out
     ```

# Custom Shell Project

This project implements a custom Unix-like shell with support for built-in commands, aliases, custom functions, I/O redirection, and more. The shell can execute standard commands, manage background and foreground processes, and provide a variety of custom features such as logging, process management, and fetching man pages from the internet.

## File Descriptions

### `color.c` and `color.h`
These files are responsible for managing the color output in the shell. Color coding is used to improve the readability of various elements like commands, files, directories, and error messages. The shell applies specific color codes for different types of output using ANSI escape sequences.

- **Error Messages**: Errors are displayed in red to ensure visibility and distinction.
- **Prompt**: The command prompt is displayed in cyan to stand out from the rest of the terminal output.
- **Files**: File names are shown in white, providing a clean and neutral display.
- **Directories**: Directories are displayed in blue, making them easily distinguishable from files.
- **Executables**: Executable file names are displayed in green, highlighting their importance.

#### Functions:
- `print_error(const char *message)`: Prints the provided error message in red.
- `print_prompt(const char *message)`: Prints the shell prompt in cyan.
- `print_file(const char *file_name)`: Prints a file name in white.
- `print_dir(const char *dir_name)`: Prints a directory name in blue.
- `print_exec(const char *exec_name)`: Prints an executable name in green.

The color codes are defined in `color.h` using ANSI escape sequences, which allow for customizing the shell's output to improve user experience.

#### Example:
When printing an error message, the function `print_error("File not found!")` will display the message in red on the terminal.


### 2. `command.c` and `command.h`
The `command.c` file is responsible for parsing and executing commands within the custom shell. It supports command execution, background processing, piping, I/O redirection, and various custom and built-in commands.

## Functions

### `process_command`

- **Purpose**: Processes and executes a command string.
- **Parameters**:
  - `const char *command`: The command string to be processed.
  - `char *home_dir`: The path to the home directory, used for certain commands.
- **Description**: 
  - Logs the command before processing.
  - Handles command aliases.
  - Splits the command string by semicolons (`;`) to manage multiple commands.
  - Identifies and manages background execution using ampersands (`&`).
  - Processes commands with pipes (`|`) and redirection operators (`<`, `>`, `>>`).
  - Executes custom functions defined in `.myshrc` or built-in commands like `hop`, `reveal`, `exit`, `neonate`, `log`, `ping`, `seek`, and `iMan`.
  - Handles process management commands (`activities`, `bg`, `fg`).
  - Updates log and process states accordingly.

## Usage

### Command Execution

- **Multiple Commands**: Commands separated by `;` are executed sequentially.
- **Background Execution**: Append `&` to a command to run it in the background. Multiple `&` can be used to specify multiple background commands.
- **Pipes**: Use `|` to pipe the output of one command into another.
- **Redirection**: Use `<`, `>`, and `>>` for input and output redirection.

### Custom Commands

- **`hop`**: Changes the directory. `hop` without arguments changes to the home directory.
- **`reveal`**: Displays files in a directory based on specified flags.
- **`neonate`**: Prints the PID of the most recently created process at intervals.
- **`seek`**: Searches for files/directories based on flags and patterns.
- **`iMan`**: Fetches and displays the man page for a specified command.

### Process Management

- **`activities`**: Lists all running processes with their states and command names.
- **`bg <pid>`**: Resumes a stopped process in the background.
- **`fg <pid>`**: Brings a background process to the foreground.

### Logging

- **`log`**: Prints the contents of the command log.
- **`log execute <index>`**: Executes a previously logged command by its index.
- **`log purge`**: Clears the command log.

### 3. `.myshrc` file
This configuration file defines custom aliases and functions for the shell. It allows users to create shortcuts and custom commands that enhance shell functionality. The file supports defining command aliases and custom functions to be used within the shell environment, providing a way to automate and streamline common tasks.

### 4. `display.c` and `display.h`
## Overview

This module handles the display of the shell prompt, including user information, system details, and the current working directory.

## Files

### `display.c`

- **`display_prompt(const char *home_dir)`**: Displays the shell prompt with user and system information. It shows:
  - Username
  - System name
  - Current working directory (relative to home directory if applicable)
  - Elapsed time for the foreground process (if greater than 2 seconds)

  The prompt includes color coding for improved visibility. The elapsed time is reset after being displayed.

### `display.h`

- **`display_prompt(const char *home_dir)`**: Function declaration for displaying the shell prompt.
- **Definitions**:
  - `MAX_PATH_LENGTH`: The maximum length of file paths.
  - `HOME_SYMBOL`: Symbol representing the home directory.
  - `HOST_NAME_MAX`: Maximum length of the system's host name.

This header file provides necessary constants and function declarations for prompt display functionality.

### 5. `hop.c` and `hop.h`
## Overview

This module provides functionality to change the current working directory, including special path handling.

## Files

### `hop.c`

- **`hop_command(char *home_dir, char *path)`**: Changes the current working directory based on the provided path. It handles:
  - `~`: Changes to the home directory.
  - `-`: Switches to the previous directory.
  - `..` and `.`: Moves up one directory level or stays in the current directory, respectively.
  - Paths starting with `~`: Resolves to directories relative to the home directory.
  - Absolute or relative paths: Directly changes to the specified path.
  
  The function also resolves the full path and prints the new directory upon successful change. It handles errors in resolving the path and changing directories.

### `hop.h`

- **`hop_command(char *home_dir, char *path)`**: Function declaration for changing the working directory.
- **Definitions**:
  - `MAX_PATH_LENGTH`: Maximum length of file paths.

This header file provides necessary constants and function declarations for directory change functionality.
### 6. `iman.c` and `iman.h`
## Overview

This module provides functionality for fetching and displaying man pages from the `man.he.net` server using an HTTP GET request.

## Files

### `iman.c`

- **`setup_connection(const char *hostname, int port_number)`**: Initializes and connects a TCP socket to the specified server. Handles socket creation, address resolution, and connection setup.
- **`fetch_man_page(const char *command_name)`**: Constructs and sends an HTTP GET request to retrieve the man page for a given command. It processes and outputs the response, stripping HTML tags from the content.

  - Connects to the `man.he.net` server on port 80.
  - Sends a request for the man page related to the provided command.
  - Reads and processes the server's response, displaying the relevant content while removing HTML tags.

### `iman.h`

- **`fetch_man_page(const char *command_name)`**: Function declaration for fetching and displaying the man page for a specified command.

This header file declares the function for retrieving man pages, and `iman.c` provides the implementation details and socket communication handling.

### 7. `linkedlist.c` and `linkedlist.h`
## Overview

This module manages a linked list of processes, allowing operations such as adding, removing, and querying processes, as well as sorting the list and retrieving process states.

## Files

### `linkedlist.c`

- **`add_process(pid_t pid, const char *command)`**: Adds a new process node to the beginning of the linked list.
- **`remove_process(pid_t pid)`**: Removes a process node with the specified PID from the linked list.
- **`get_process_list_head()`**: Returns the head of the process linked list.
- **`get_process_name(pid_t pid)`**: Retrieves the command name associated with a process PID. Returns "Unknown" if the PID is not found.
- **`free_process_list()`**: Frees all nodes in the linked list and sets the head to NULL.
- **`get_process_state(pid_t pid)`**: Fetches the state of a process (e.g., Running, Stopped) by reading from the `/proc/[pid]/status` file.
- **`sort_process_list()`**: Sorts the linked list of processes by command name in lexicographic order using the `qsort` function.

### `linkedlist.h`

- **`ProcessNode`**: Defines the structure for a process node, containing the PID, command name, and a pointer to the next node.
- **`add_process(pid_t pid, const char *command)`**: Function declaration to add a process to the list.
- **`remove_process(pid_t pid)`**: Function declaration to remove a process from the list.
- **`get_process_name(pid_t pid)`**: Function declaration to retrieve the command name for a process PID.
- **`free_process_list()`**: Function declaration to free the entire list.
- **`get_process_state(pid_t pid)`**: Function declaration to get the current state of a process.
- **`sort_process_list()`**: Function declaration to sort the process list by command name.
- **`get_process_list_head()`**: Function declaration to get the head of the process linked list.

This module provides essential functionality for process management in a linked list, supporting process addition, removal, state retrieval, and list sorting.

### 8. `log.c` and `log.h`
## Overview

This module provides functionalities for logging commands, managing log files, and retrieving historical commands. It supports setting the log directory, initializing and cleaning up logging resources, and handling log file operations such as trimming, purging, and querying.

## Files

### `log.c`

- **`set_log_directory(const char *home_dir)`**: Sets the directory for log files based on the provided home directory. Creates the log directory if it does not exist. Constructs the full path to the log file.
- **`init_log()`**: Initializes logging by setting the `last_command` to NULL.
- **`cleanup_log()`**: Frees resources related to logging, particularly the `last_command` string.
- **`trim_log_file()`**: Trims the log file to keep only the most recent `MAX_LOG_ENTRIES` entries. Reads all lines into a dynamic list, then writes the most recent entries back to the log file.
- **`log_command(const char *command)`**: Logs a new command if it is not a duplicate of the last command or does not contain "log". Updates the `last_command` and appends the command to the log file.
- **`print_log()`**: Prints the contents of the log file to the standard output.
- **`log_purge()`**: Clears the contents of the log file, effectively purging it.
- **`get_command_from_log(int index)`**: Retrieves a command from the log by its index (from the end of the log). Returns the command as a string or NULL if the index is invalid.

### `log.h`

- **`set_log_directory(const char *home_dir)`**: Function declaration to set the log directory.
- **`init_log()`**: Function declaration to initialize logging.
- **`cleanup_log()`**: Function declaration to clean up logging resources.
- **`trim_log_file()`**: Function declaration to trim the log file.
- **`log_command(const char *command)`**: Function declaration to log a new command.
- **`print_log()`**: Function declaration to print the log contents.
- **`log_purge()`**: Function declaration to purge the log file.
- **`get_command_from_log(int index)`**: Function declaration to retrieve a command by index from the log.
- **`MAX_LOG_ENTRIES`**: Defines the maximum number of log entries to keep.
- **`LOG_FILE`**: Defines the log file name (without directory).

## Usage

1. **Setting Log Directory**: Call `set_log_directory()` with the home directory to set up the log directory and file path.
2. **Initializing Logging**: Call `init_log()` to initialize logging.
3. **Logging Commands**: Use `log_command()` to log commands executed.
4. **Printing Log**: Call `print_log()` to view the log contents.
5. **Trimming Log File**: Automatically done by `log_command()` to maintain a fixed number of recent entries.
6. **Purging Log**: Call `log_purge()` to clear the log file.
7. **Retrieving Commands**: Use `get_command_from_log()` to retrieve specific commands from the log based on index.

This module provides essential functionality for command logging, with capabilities to manage and maintain a history of commands in a structured log file.

### 9. `main.c`
## Overview

This file contains the main driver code for the shell application. It sets up the shell environment, handles user input, and manages logging. The main loop continuously prompts the user for input, processes commands, and maintains the logging of commands.

## Functions

### `int main()`

1. **Retrieve Home Directory**:
   - Uses `getcwd()` to get the current working directory, which is assumed to be the home directory.
   - If `getcwd()` fails, it prints an error message and exits.

2. **Load Aliases and Functions**:
   - Calls `load_aliases(".myshrc")` to load command aliases from the `.myshrc` file.
   - Calls `load_functions(".myshrc")` to load custom functions defined in the `.myshrc` file.

3. **Initialize Logging**:
   - Sets the log directory using `set_log_directory(home_dir)`.
   - Initializes logging with `init_log()`.

4. **Setup Signal Handlers**:
   - Calls `setup_signal_handlers()` to set up signal handling for the shell.

5. **Main Loop**:
   - Continuously prompts the user for input using `display_prompt(home_dir)`.
   - Reads user input with `fgets()`. If `fgets()` returns NULL due to EOF (Ctrl-D), it handles it by calling `handle_sigquit(SIGQUIT)` to handle logging out.
   - Processes the command using `process_command(command, home_dir)`.
   - Strips the newline character from the input command.

6. **Cleanup**:
   - Cleans up logging resources with `cleanup_log()` before exiting.

## Error Handling

- If `getcwd()` fails, an error message is printed, and the program exits with a failure status.
- If `fgets()` fails due to reasons other than EOF, an error message is printed, and the program exits with a failure status.

## Dependencies

This file relies on the following header files and their implementations:

- `display.h` - For displaying the shell prompt.
- `hop.h` - For handling the `hop` command functionality.
- `reveal.h` - For handling the `reveal` command functionality.
- `log.h` - For logging commands and managing the log file.
- `signal.h` - For setting up signal handlers.
- `custom.h` - For loading and executing custom functions.
- `command.h` - For processing commands.


### 10. `neonate.c` and `neonate.h`
## Overview

The `neonate` command is a utility that prints the PID of the most recently created process every specified number of seconds. It continues to run until interrupted by pressing the 'x' key.

## Files

- **`neonate.c`**: Contains the implementation of the `neonate` command.
- **`neonate.h`**: Header file for the `neonate` function prototype.

### 11. `proclore.c` and `proclore.h`
## Overview

The `proclore` command provides detailed information about a process identified by its PID. If no PID is specified, it defaults to providing information about the current process.

## Files

- **`proclore.h`**: Header file containing the function prototype for `proclore`.
- **`proclore.c`**: Source file containing the implementation of the `proclore` function.

### 12. `reveal.c` and `reveal.h`
The `reveal` command is a custom directory listing utility designed to display files and directories with support for various flags and color-coded output. It provides functionalities similar to the `ls` command in Unix-like systems.

## Files

- **`reveal.h`**: Header file with the function prototype for `reveal_command`.
- **`reveal.c`**: Source file containing the implementation of the `reveal_command` function.

### 13. `seek.c` and `seek.h`
The `seek` command is a custom utility designed to search for files and directories based on a search term. It offers various options to filter search results, display files or directories, and handle exact or partial matches. Additionally, if an exact match is found, it performs specific actions based on whether the result is a directory or a file.

## Files

- **`seek.c`**: Source file containing the implementation of the `seek_command_handler` function.
- **`seek.h`**: Header file with the function prototype for `seek_command_handler`.

### 14. `signal.c` and `signal.h`
## `signal.h`

Header file declaring the signal handling functions and the `foreground_pid` variable used for process management.

## `signal.c`

### Overview

Implements the signal handling functions declared in `signal.h`. Manages process control and signal responses for a shell application.

### Key Functions

- **`void send_signal(pid_t pid, int signal_number);`**
  - Sends a signal to a process and handles errors if the process does not exist.

- **`void handle_sigint(int signum);`**
  - Handles SIGINT (Ctrl-C) by sending an interrupt signal to the foreground process, if it exists.

- **`void handle_sigquit(int signum);`**
  - Handles SIGQUIT (Ctrl-D) by terminating all running processes and exiting the shell.

- **`static void background_process_handler(int sig);`**
  - Manages background processes, waits for their termination, and updates the process list.

- **`void handle_sigtstp(int signum);`**
  - Handles SIGTSTP (Ctrl-Z) by stopping the foreground process, moving it to the background, and invoking the background process handler.

- **`void setup_signal_handlers();`**
  - Configures signal handlers for SIGCHLD, SIGINT, SIGQUIT, and SIGTSTP to ensure proper handling of process control signals.
