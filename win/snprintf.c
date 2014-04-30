#include <stdio.h>
#include <stdarg.h>

int snprintf(char *buffer, size_t count, const char *format, ...)
{
    int len;
    va_list args;
    va_start(args, format);
    len = vsnprintf(buffer, count, format, args);
    va_end(args);
    if (len < 0) {
        len = count + 1;
    }
    return len;
}

