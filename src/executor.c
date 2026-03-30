#include "../include/executor.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>


void create_child_process_and_wait(char *full_path, char **tokens) {
  pid_t pid = fork();

  if (pid < 0)
    perror("Fork Failed !!");
  else if (pid == 0) {
    // Child process is running
    // use execv to replace current process image with new one
    if (execv(full_path, tokens) == -1) {
      perror("execv failed !!");
      exit(1); // Kill the child since execv failed.
    }
  } else {
    // Parent process is running
    // Wait until the child process finishes

    int status; // bitmask that will contain info about how the child died
    waitpid(pid, &status, 0);
  }
}

char *find_command_in_path(char *command) {
  // Get the path variable
  char *path_env = getenv("PATH");
  if (path_env == NULL || strlen(path_env) == 0)
    return NULL;

  // getenv returns the pointer to actual path value string
  // It is extremely dangerous to mess with the actual path
  // so create a copy
  char *path_env_copy = strdup(path_env);

  // tokenize the path
  char *dir = strtok(path_env_copy, ":");
  while (dir != NULL) {
    // Construct the possible path string as dir + / + command + \0
    size_t path_len = strlen(dir) + strlen(command) + 2; //+2 for / and \0
    char *full_path = malloc(path_len);
    snprintf(full_path, path_len, "%s/%s", dir, command);

    // Check if the constructed path string is valid and executable
    if (access(full_path, X_OK) == 0) {
      // We found an executable.. free the memory and return the path
      free(path_env_copy);
      return full_path;
    } else {
      // Executable not found in this dir.. free up and move to next
      free(full_path);
      dir = strtok(NULL, ":");
    }
  }
  // If we exhausted the PATH variable and still did not find anything..
  // free up and return NULL
  free(path_env_copy);
  return NULL;
}
