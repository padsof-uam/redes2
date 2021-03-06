#include "strings.h"

#include <ctype.h>
#include <string.h>

void strip(char **str)
{
    if (!str || !(*str))
        return;

    while (isspace(**str))
        (*str)++;

    strip_end(*str);
}

void strip_end(const char* str)
{
    char* end;

    if (!str || !(*str))
        return;

    end = strchr(str, '\0');
    end--;

    while (isspace(*end))
    {
        *end = '\0';
        end--;
    }   
}

char * strnstr(const char* s, const char* find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if ((sc = *s++) == '\0' || slen-- < 1)
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}


int str_arrsep(char* str, const char* sep, char** array, size_t arrlen)
{
    char* token;
    int i = 0;

    while((token = strsep(&str, sep)) != NULL && i < arrlen)
    {
        array[i] = token;
        i++;
    }

    return i;
}
