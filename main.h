#ifndef MAIN_H
#define MAIN_H

#ifdef _WIN32

#define strcasecmp _stricmp
#define strlwr _strlwr
#define strupr _strupr

#else

#include <cctype>
char * strlwr(char * string);
char * strupr(char * string);

#ifdef MAIN_STRLWR_STRUPR_IMPLEMENTATION
char * strlwr(char * string) {
    int length = strlen(string);
    for (int i = 0; i < length; ++i)
    {
        string[i] = tolower(string[i]);
    }
    return string;
}
char * strupr(char * string) {
    int length = strlen(string);
    for (int i = 0; i < length; ++i)
    {
        string[i] = toupper(string[i]);
    }
    return string;
}
#endif

#endif

#endif