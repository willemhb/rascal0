#ifndef eval_h
#define eval_h
#include "rascal.h"
#include "object.h"


/* Forward declarations */
lobj_t * lobj_eval(lobj_t *, lobj_t **);
lobj_t * lobj_qeval(lobj_t *, lobj_t **);
lobj_t * apply_lambda(lambda_t *, lobj_t *);
lobj_t * apply_prim(prim_t *, lobj_t **, lobj_t *);
lobj_t * apply_form(form_t *, lobj_t **, lobj_t *);
lobj_t * apply(lobj_t *, lobj_t **, lobj_t *);

#endif
