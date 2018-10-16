#ifndef evaluator_h
#define evaluator_h

#include "environment.h"
#include "sexpr.h"

sexpr* eval2(scheme_env*, sexpr*);
void load_built_ins(scheme_env*);

#define IS_FUNC(f) (f->type == LVAL_LIST && f->count > 0) ? 1 : 0
#define ASSERT_PRIMITIVE(e, s) sexpr *c = env_fetch_var(e, s); \
            if (c->type == LVAL_FUN && c->builtin) { \
                sexpr_free(c); \
                return sexpr_err("Scheme primitives cannot be redefined."); }

#endif

#define ASSERT_PARAM_MIN(c, e, err) if (c < e) return sexpr_err(err)
#define ASSERT_PARAM_EQ(c, e, err) if (c != e) return sexpr_err(err)

#define IS_ELSE_CLAUSE(n, c, cond) n == (c - 1) && cond->type == LVAL_SYM \
                                    && strcmp("else", cond->sym) == 0

typedef int(*cmpf)(float x, float y);
