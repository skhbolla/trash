#include "../include/builtins.h"
#include "../include/executor.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int handle_builtins(char **tokens, int token_count) {
  // Handle builtins
  if (strcmp(tokens[0], "exit") == 0) {
    return 2; // Signal main loop to break
  }

  else if (strcmp(tokens[0], "echo") == 0) {
    // Iterate through the tokens starting from index 1
    for (int i = 1; i < token_count; i++) {
      printf("%s", tokens[i]);
      if (i != token_count - 1)
        printf(" ");
    }
    printf("\n"); // echo always ends in new line
    return 1;
  }

  else if (strcmp(tokens[0], "type") == 0) {
    for (int i = 1; i < token_count; i++) {
      if (strcmp(tokens[i], "echo") == 0 || strcmp(tokens[i], "type") == 0 ||
          strcmp(tokens[i], "exit") == 0 || strcmp(tokens[i], "pwd") == 0) {
        printf("%s is a shell builtin\n", tokens[i]);
      } else {
        char *path = find_command_in_path(tokens[i]);
        if (path == NULL) {
          printf("%s: not found\n", tokens[i]);
        } else {
          printf("%s is %s\n", tokens[i], path);
          free(path); // Essential cleanup for the malloc in find_command_in_path
        }
      }
    }
    return 1;
  }

  else if (strcmp(tokens[0], "pwd") == 0) {
    char pwd_path[2048];
    char *x = getcwd(pwd_path, sizeof(pwd_path));

    if (x == NULL) { // This should never happen because kernel guarentees
                     // PATH_MAX will be the longest possible
      printf("cwd path is too long!!\n");
    } else {
      printf("%s\n", x);
    }
    return 1;
  }

  else if (strcmp(tokens[0], "cd") == 0) {
    char *target = NULL;

    if (token_count > 2) {
      fprintf(stderr, "cd: too many arguments\n");
      return 1;
    }

    else if (token_count == 1) {
      // No args passed - move to HOME
      target = getenv("HOME");

      if (target == NULL || strlen(target) == 0) {
        fprintf(stderr, "HOME not set\n");
        return 1;
      }
    }

    else {
      if (tokens[1][0] == '~') {
        // Tilde expansion logic here
        char *home = getenv("HOME");
        if (home == NULL || strlen(home) == 0) {
          fprintf(stderr, "HOME not set\n");
          return 1;
        }
        char expanded_path[2048];
        snprintf(expanded_path, 2048, "%s%s", home, tokens[1] + 1);
        target = expanded_path;
      } else
        target = tokens[1];
    }

    if (chdir(target) != 0) {
        printf("cd: %s: No such file or directory\n", target);
    }
    return 1;
  }
  return 0; // Not a builtin
}


