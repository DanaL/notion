#ifndef evaluator_h
#define evaluator_h

#include "environment.h"
#include "sexpr.h"

sexpr* eval(scheme_env*, sexpr*);
int is_built_in(sexpr*);

#define IS_FUNC(f) (f->type == LVAL_LIST && f->count > 0) ? 1 : 0

#endif
