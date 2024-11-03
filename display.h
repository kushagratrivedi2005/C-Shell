#ifndef DISPLAY_H
#define DISPLAY_H

#include <limits.h>

#define MAX_PATH_LENGTH PATH_MAX
#define HOME_SYMBOL "~"

// Define HOST_NAME_MAX if not defined
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64  // Default value if HOST_NAME_MAX is not defined
#endif

// Function declarations
void display_prompt(const char *home_dir);

#endif // DISPLAY_H
