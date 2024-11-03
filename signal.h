#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>
#include <sys/types.h>

extern pid_t foreground_pid;
// Function to send a signal to a process
void send_signal(pid_t pid, int signal_number);

// Function to handle Ctrl-C (SIGINT)
void handle_sigint(int signum);

// Function to handle Ctrl-D (logout)
void handle_sigquit(int signum);

// Function to handle Ctrl-Z (SIGTSTP)
void handle_sigtstp(int signum);

// Function to set up signal handlers
void setup_signal_handlers();

#endif
