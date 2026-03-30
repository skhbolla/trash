#ifndef REDIRECTION_H
#define REDIRECTION_H

int setup_redirection(int *token_count, char **tokens, int saved_fds[3]);
void restore_redirection(int saved_fds[3]);

#endif
