#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>

// Global flag to toggle debug output
extern int TRACE_MODE;

void helpers_init();

void print_banner();

// The helper function (using variadic arguments)
void trace_print(const char *format, ...);

void trace_hex_buffer(const char *buf, ssize_t len);

void trace_tokens(char **buf, ssize_t len);

#endif
