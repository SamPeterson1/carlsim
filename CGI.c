#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_SIZE 32
#define VALUE_SIZE 32
#define MAX_QUERY_LENGTH 64
#define MAX_KEYVALUE_PAIRS 16
#define RETURN_FAILURE 0
#define RETURN_SUCCESS 1

typedef struct
{
    char key[KEY_SIZE];
    char value[VALUE_SIZE];
} KeyValuePair;

int getKeyValuePairs(KeyValuePair *p_, int *n_);

int getKeyValuePairs(KeyValuePair *p_, int *n_)
{
    char *q = NULL; /* Query String */
    char c = 0;     /* Single Character from Query String */
    int i = 0;      /* Current Index in Query String */
    int j = 0;      /* Current Index in KeyValuePair Array */
    int k = 0;      /* In Key (0) or Value (1) */
    int l = 0;      /* Length of Query String */
    int m = 0;      /* Current Index in Key or Value String */

    int ret = RETURN_FAILURE;

    q = getenv("QUERY_STRING");

    if (NULL != q)
    {
        l = strnlen(q, MAX_QUERY_LENGTH);

        if (0 < l)
        {
            for (i = 0; i < l; i++)
            {
                c = q[i];
                if (38 == c) /* & */
                {
                    j++;   /* Move to the next key value pair in the array */
                    k = 0; /* We are now in the key */
                    m = 0; /* Reset our index in the key string */
                }
                else if (61 == c) /* = */
                {
                    k = 1; /* We are now in the value */
                    m = 0; /* Reset our index in the value string */
                }
                else if ((32 < c) && (127 > c))
                {
                    if (0 == k) /* We are in the key */
                    {
                        p_[j].key[m] = c; /* Add the character from the query string to the key */
                        m++;
                    }
                    else if (1 == k) /* We are in the value */
                    {
                        p_[j].value[m] = c; /* Add the character from the query string to the value */
                        m++;
                    }
                }
            }
            *n_ = j + 1;

            ret = RETURN_SUCCESS;
        }
    }

    return ret;
}