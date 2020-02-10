#ifndef eval_h
#define eval_h
#include "rascal.h"
#include "object.h"


/* Forward declarations */
lobj_t * lobj_eval(lobj_t *, sym_t *);
lobj_t * lobj_qeval(lobj_t *, sym_t *);
lobj_t * apply_lambda(lambda_t * fun, lobj_t * args);
lobj_t * apply_prim(prim_t * fun, sym_t * env, lobj_t * args);
lobj_t * apply(lobj_t * fun, sym_t * env, lobj_t * args);

#endif
