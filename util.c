#include <ctype.h>
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
    memcpy(s, ct, sz + 1);
#else
    int sz = strlen(ct) + 1;
    s = malloc(sz);
    strlcpy(s, ct, sz);
#endif

    return s;
}

int is_whitespace(char *s) {
	while (*s) {
		if (!isspace(*s++))
			return 0;
	}

	return 1;
}
