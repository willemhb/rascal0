#ifndef eval_h
#define eval_h
#include "rascal.h"
#include "object.h"
#include "util.h"
#include "printer.h"

/* Forward declarations */
lobj_t * lobj_eval(lobj_t *, lobj_t **);
lobj_t * lobj_expand(lobj_t *, lobj_t **);
lobj_t * apply_lambda(lobj_t *, lobj_t *);
lobj_t * apply_prim(lobj_t *, lobj_t **, lobj_t *);
lobj_t * apply(lobj_t *, lobj_t **, lobj_t *);
lobj_t * bind_args(lambda_t *, lobj_t *);

#endif
