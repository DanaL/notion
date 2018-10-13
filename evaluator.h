#ifndef evaluator_h
#define evaluator_h

#include "environment.h"
#include "sexpr.h"

sexpr* eval(scheme_env*, sexpr*);

#endif
