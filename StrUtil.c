#include "StrUtil.h"

char **initBuffer(void)
{

    char **ret;

    ret = (char **)calloc(1, MAX_ARG_COUNT * sizeof(char *));
    *(ret + 0) = (char *)calloc(1, MAX_ARG_COUNT * (MAX_ARG_LENGTH + 1) * sizeof(char));
    for (int i = 0; i < MAX_ARG_COUNT; i++)
    {
        *(ret + i) = (*ret + (MAX_ARG_LENGTH + 1) * i);
    }

    return ret;
}

void parse(const char *str, char **args, int *argc)
{
    int pos = 0;
    int i = 0;
    int ch = 0;
    int len = 0;
    int intoken = TRUE;
    int inliteral = FALSE;

    *argc = 0;
    memset(*(args + 0), 0, MAX_ARG_COUNT * (MAX_ARG_LENGTH + 1) * sizeof(char));

    len = strlen(str);

    for (i = 0; i < len; i++)
    {
        ch = str[i];
        if (ch == 32)
        {
            if (inliteral)
            {
                *(args[*argc] + pos) = ch;
                pos += 1;
            }
            else
            {
                if (intoken)
                {
                    intoken = FALSE;
                    inliteral = FALSE;
                    pos = 0;
                }
            }
        }
        else if (ch == 34)
        {
            if (inliteral)
            {
                intoken = FALSE;
                inliteral = FALSE;
                pos = 0;
            }
            else
            {
                intoken = TRUE;
                inliteral = TRUE;
                *argc += 1;
                pos = 0;
            }
        }
        else if (ch >= 33 && ch <= 126)
        {

            if (intoken)
            {
                *(args[*argc] + pos) = ch;
                pos += 1;
            }
            else
            {
                *argc += 1;
                pos = 0;
                *(args[*argc] + pos) = ch;
                pos += 1;
            }
            intoken = TRUE;
        }
    }

    *argc += 1;
}