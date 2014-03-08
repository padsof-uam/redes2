#include "strings.h"

#include <ctype.h>
#include <string.h>

void strip(char **str)
{
    char *end;

    if (!str || !(*str))
        return;

    while (isspace(**str))
        (*str)++;

    end = strchr(*str, '\0');
    end--;

    while (isspace(*end))
    {
        *end = '\0';
        end--;
    }
}


