#ifndef util_h
#define util_h

/* A place for functions and such that don't really fit in thematically with
  the other units */

/* strlcpy doesn't exist to Visual Studio's compiler and strcpy_s doesn't
  seem to exist in BSD-land so inside the function I can conditionally
  comppile to the appropriate "safe" function */
char* n_strcpy(char*, char*);

int is_whitespace(char*);

#endif
