#ifndef REVEAL_H
#define REVEAL_H

#include <stdbool.h>
#include <limits.h>

#define MAX_PATH_LENGTH PATH_MAX

// Function declaration for the reveal command
void reveal_command(const char *flags, const char *path, const char *home_dir);

#endif // REVEAL_H
