#include "../include/parser.h"
#include <string.h>

#define MAX_TOKENS 64

int tokenize_input(char *raw_input, char **tokens) {
  // Tokenize the input
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
