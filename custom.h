#ifndef CUSTOM_H
#define CUSTOM_H

// Loads the function definitions from the .myshrc file
void load_functions(const char *myshrc_file);

// Executes the command if it matches a function defined in the .myshrc file
int execute_custom_function(const char *command,char * home);

#endif // CUSTOM_H
