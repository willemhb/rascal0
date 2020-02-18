#ifndef util_h

#define util_h
#include "rascal.h"
#include "object.h"

typedef struct _tuple_t {
  int len;
  lobj_t ** values;
} tuple_t;

tuple_t * new_tuple(int);
int list_len(lobj_t *);
tuple_t * list_to_tuple(lobj_t *);
lobj_t ** getargs(lobj_t *, lobj_t *);

#endif
