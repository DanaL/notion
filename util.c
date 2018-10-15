#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"

char* n_strcpy(char *s, char *ct) {

#ifdef _WIN32
    /* MingW on Windows didn't include strlcpy (which may be a BSDland thing?),
        and I read so posts pooh-poohing strncpy so I'll just go with this. */
    int sz = strlen(ct);
    s = malloc(sz + 1);
    memcpy(s, ct, sz);
    s[sz] = '\0';
#else
    int sz = strlen(ct) + 1;
    s = malloc(sz);
    strlcpy(s, ct, sz);
#endif

    return s;
}
