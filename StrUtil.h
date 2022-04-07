#include "Defines.h"
#include <string.h>
#include <stdlib.h>

#define MAX_ARG_COUNT 64
#define MAX_ARG_LENGTH 256

char **initBuffer();
void parse(const char *str, char **args, int *argc);