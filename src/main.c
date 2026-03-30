#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //needed for read() and STDIN_FILENO
#include "../include/parser.h"
#include "../include/executor.h"
#include "../include/redirection.h"
#include "../include/builtins.h"

#define MAX_INPUT 1024
#define MAX_TOKENS 64


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
    int saved_fds[3] = {-1, -1, -1}; // Standard 0=in 1=out 2=err
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
