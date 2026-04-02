#include <stdio.h>
#include <stdarg.h>
#include "../include/helpers.h"

int TRACE_MODE = 0;

void trace_print(const char *format, ...) {
    if(!TRACE_MODE) return;

    va_list args;
    va_start(args, format);

    // Prefixing with [TRACE] makes it easy to grep/filter logs
    fprintf(stderr, "[TRACE] ");
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n"); 

    va_end(args);
}

void trace_hex_buffer(const char *buf, ssize_t len) {
    if (!TRACE_MODE) return;

    fprintf(stderr, "[TRACE HEX]");
    for (int i = 0; i < len ; i++) {
        fprintf(stderr, "%02x ", (unsigned char)buf[i]);
        // unisgned char is necessary because If you happen to read a byte 
        // that has the high bit set (like an emoji or a special symbol), 
        // C might "sign-extend" it, turning a simple 0xFF into 0xFFFFFFFF when 
        // printing. Casting to unsigned char ensures you only see the clean, 
        // 2-digit hex code for that specific byte.
    }
    fprintf(stderr, "\n");
}

void trace_tokens(char **buf, ssize_t len) {
    fprintf(stderr, "[TRACE TOKENS]");
    for (int i = 0; i < len; i++) {
        fprintf(stderr, "[%s], " , buf[i]);
    }
    fprintf(stderr, "\n");
}
