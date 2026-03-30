#include "../include/redirection.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


int setup_redirection(int *token_count, char **tokens, int saved_fds[3]) {
    // Loop through the tokens and look for redirection symbols
    for (int i = 0; i < *token_count; i++) {
        if (strcmp(tokens[i], ">") == 0 ||
                strcmp(tokens[i] , "1>") == 0 ||
                strcmp(tokens[i], ">>") == 0 ||
                strcmp(tokens[i], "1>>") == 0) {
            char *filename = tokens[i+1];
            if (filename == NULL) {
                fprintf(stderr, "Unexpected token newline\n");
                return -2;
            } else {
                // take a backup of original fd to restore later
                saved_fds[1] = dup(STDOUT_FILENO);
                int new_fd = -1;
                // Open the file
                if (strcmp(tokens[i], ">>") == 0 || strcmp(tokens[i], "1>>") == 0) {
                    new_fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                }
                else {
                    new_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
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
        else if (strcmp(tokens[i] , "2>") == 0 ||
                strcmp(tokens[i], "2>>") == 0) {
            char *filename = tokens[i+1];
            if (filename == NULL) {
                fprintf(stderr, "Unexpected token newline\n");
                return -2;
            } else {
                // take a backup of original fd to restore later
                saved_fds[2] = dup(STDERR_FILENO);
                int new_fd = -1;

                // Open the file
                if (strcmp(tokens[i], "2>>") == 0) {
                    new_fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                }
                else {
                    new_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
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


