#ifndef COLOR_H
#define COLOR_H

// Color codes
#define RESET       "\x1B[0m"
#define RED         "\x1B[31m"
#define GREEN       "\x1B[32m"
#define YELLOW      "\x1B[33m"
#define BLUE        "\x1B[34m"
#define MAGENTA     "\x1B[35m"
#define CYAN        "\x1B[36m"
#define WHITE       "\x1B[37m"

// Specific color coding
#define ERROR_COLOR RED
#define PROMPT_COLOR CYAN
#define FILE_COLOR WHITE
#define DIR_COLOR BLUE
#define EXEC_COLOR GREEN

// Function prototypes
void print_error(const char *message);
void print_prompt(const char *message);
void print_file(const char *file_name);
void print_dir(const char *dir_name);
void print_exec(const char *exec_name);

#endif // COLOR_H
