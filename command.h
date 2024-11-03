#ifndef COMMAND_H
#define COMMAND_H
extern double elapsed_time;

void process_command(const char *command, char *home_dir);
void load_aliases(const char *filename);

#endif
