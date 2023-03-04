#include "Log.h"

void clearLog(void) {
    FILE *l = fopen("Log/log.txt", "w");
    fclose(l);
}

void lprintf(char *format, ...) {
    
    FILE *l = fopen("Log/log.txt", "a");
    va_list args;
    va_start(args, format);
    vfprintf(l, format, args);
    va_end(args);
    fclose(l);
    
}
