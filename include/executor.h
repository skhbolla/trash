#ifndef EXECUTOR_H
#define EXECUTOR_H

void create_child_process_and_wait(char *full_path, char **tokens);
char *find_command_in_path(char *command);

#endif
