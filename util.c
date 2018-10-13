#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"

char* n_strcpy(char *s, char *ct) {
  int sz = strlen(ct) + 1;
  s = malloc(sz);
  strlcpy(s, ct, sz);

  return s;
}
