#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h> //needed for read() and STDIN_FILENO
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_TOKENS 64

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

int tokenize_input(char *raw_input, char **tokens) {
  // Tokenize the input
  // Manual Tokenizer replacement for strtok to handle quotes and backslashes
  int token_count = 0;
  char *p = raw_input;
  char quote_type = '\0';

  while (*p != '\0' && token_count < MAX_TOKENS - 1) {
    while (*p == ' ' && quote_type == '\0') p++;
    if (*p == '\0') break;

    tokens[token_count++] = p;

    while (*p != '\0') {
      // Handle Backslashes: Only escape if NOT in single quotes
      if (*p == '\\' && quote_type != '\'') {
        memmove(p, p + 1, strlen(p));
        if (*p != '\0') {
          p++;
        }
        continue;
      }

      if (*p == '\'' || *p == '\"') {
        if (quote_type == '\0') {
          quote_type = *p;
          memmove(p, p + 1, strlen(p));
          continue;
        } else if (quote_type == *p) {
          quote_type = '\0';
          memmove(p, p + 1, strlen(p));
          continue;
        }
      }
      if (*p == ' ' && quote_type == '\0') {
        *p = '\0';
        p++;
        break;
      }
      p++;
    }
  }
  tokens[token_count] = NULL;
  return token_count;
}

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

int setup_redirection(int *token_count, char **tokens, int saved_fds[3]) {
    // Loop through the tokens and look for redirection symbols
    for (int i = 0; i < *token_count; i++) {
        if (strcmp(tokens[i], ">") == 0 || strcmp(tokens[i] , "1>") == 0) {
            char *filename = tokens[i+1];
            if (filename == NULL) {
                fprintf(stderr, "Unexpected token newline\n");
                return -2;
            } else {
                // take a backup of original fd to restore later
                saved_fds[1] = dup(STDIN_FILENO);

                // Open the file
                int new_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(new_fd < 0) {
                    perror("open");
                    return -2;
                }

                //Replace original fd with new fd
                dup2(new_fd, STDOUT_FILENO);
                close(new_fd);

                //Cleanup tokens
                tokens[i] = NULL;
                *token_count = i;

                break;
            }
        }
        else if (strcmp(tokens[i] , "2>") == 0) {
            char *filename = tokens[i+1];
            if (filename == NULL) {
                fprintf(stderr, "Unexpected token newline\n");
                return -2;
            } else {
                // take a backup of original fd to restore later
                saved_fds[2] = dup(STDERR_FILENO);

                // Open the file
                int new_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(new_fd < 0) {
                    perror("open");
                    return -2;
                }

                //Replace original fd with new fd
                dup2(new_fd, STDERR_FILENO);
                close(new_fd);

                //Cleanup tokens
                tokens[i] = NULL;
                *token_count = i;

                break;
            }
        }

    }
    return 0;
}

void restore_redirection(int saved_fds[3]) {
    for (int i = 0; i < 3; i++) {
        if (saved_fds[i] != -1) {
            dup2(saved_fds[i], i);
            close(saved_fds[i]);
            saved_fds[i] = -1;
        }
    }
}


int main(int argc, char *argv[]) {
  // setbuf toggles the stream's buffering mode
  // By default stdout is line buffered or block buffered,
  // Since we need to print stuff to screen without having to wait for a full
  // block or a \n char , we can set the stdout to no buffer at all (NULL)
  // setbuf acts like a global toggle to disable stdout buffering..
  // Instead we can use fflush() to manually flush the buffer after each printf
  // too..
  setbuf(stdout, NULL);

  char raw_input[MAX_INPUT];
  char copy_input[MAX_INPUT]; // tokenization destroys the raw_input,
                              // so maintain a copy
  char *tokens[MAX_TOKENS];   // Array that will contain pointers to tokens

  while (1) {

    // Display the Prompt
    printf("$ ");

    // Read user input
    // read() return number of bytes read; 0 for EOF ; -1 for Error
    ssize_t n = read(STDIN_FILENO, raw_input, MAX_INPUT - 1);

    if (n == 0) {
      printf("EOF encountered .. Exit triggered\n");
      break;
    } else if (n < 0) {
      perror("read failed\n");
      break;
    } else {
      // i.e read() returned atleast 1 byte

      // Since read() does not automatically add a NULL terminator like
      // fgets(), we need to manually add it
      raw_input[n] = '\0';

      // Clean NewLine
      // If the last char is \n remove it so tokens are not messed up
      if (raw_input[n - 1] == '\n') {
        raw_input[n - 1] = '\0';
      }
    }

    // Keep a copy of original input
    strcpy(copy_input, raw_input);

    // -----------------------------------------------------------
    // Use the tokenizer function
    // -----------------------------------------------------------
    int token_count = tokenize_input(raw_input, tokens);

    // Handle empty command
    // i.e user inputs \n only
    if (tokens[0] == NULL)
      continue;

    // -----------------------------------------------------------
    // Handle redirection
    // -----------------------------------------------------------
    int saved_fds[3]; // Standard 0=in 1=out 2=err
    if (setup_redirection(&token_count, tokens, saved_fds) == -2) {
        fprintf(stderr, "Error setting up redirection");
        continue;
    }

    // -----------------------------------------------------------
    // Use the builtins function
    // -----------------------------------------------------------
    int builtin_status = handle_builtins(tokens, token_count);
    if (builtin_status == 2) {
        restore_redirection(saved_fds);
        break; // exit command called
    } else if (builtin_status == 1) {
        restore_redirection(saved_fds);
        continue; // command was a builtin and successfully executed
    } else {
      // Check if executable exists in PATH
      char *path = find_command_in_path(tokens[0]);

      if (path != NULL) {
        // executable found ... fork and exec
        create_child_process_and_wait(path, tokens);
        free(path);
      } else {
        printf("%s: command not found\n", tokens[0]);
      }

      restore_redirection(saved_fds);
    }
  }

  return 0;
}
