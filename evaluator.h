#ifndef evaluator_h
#define evaluator_h

#include "environment.h"
#include "sexpr.h"

sexpr* eval2(vm_heap*, scope*, sexpr*);
void load_built_ins(scope*);

#define IS_FUNC(f) (f->type == LVAL_LIST && f->count > 0) ? 1 : 0
#define ASSERT_PRIMITIVE(vm, e, s) sexpr *c = scope_fetch_var(vm, e, s); \
            if (c->type == LVAL_FUN && c->builtin) { \
                return sexpr_err(vm, "Scheme primitives cannot be redefined."); }

#endif

#define ASSERT_PARAM_MIN(c, e, err) if (c < e) return sexpr_err(vm, err)
#define ASSERT_PARAM_EQ(c, e, err) if (c != e) return sexpr_err(vm, err)
#define ASSERT_NOT_ERR(e) if (e->type == LVAL_ERR) return e
#define ASSERT_TYPE(e, t, err) if (e->type != t) return sexpr_err(vm, err)

#define IS_ELSE_CLAUSE(n, c, cond) n == (c - 1) && cond->type == LVAL_SYM \
                                    && strcmp("else", cond->sym) == 0

typedef int(*cmpf)(float x, float y);
