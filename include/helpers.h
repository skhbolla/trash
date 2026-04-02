#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>

// Global flag to toggle debug output
extern int TRACE_MODE;

// The helper function (using variadic arguments)
void trace_print(const char *format, ...);

void trace_hex_buffer(const char *buf, ssize_t len);

void trace_tokens(char **buf, ssize_t len);

#endif
