#ifndef util_h
#define util_h
#include "rascal.h"
#include "object.h"


/*
Utilities for working on the backend.
 */

// Get the length of a lisp list as a c size_t
size_t list_len(lobj_t * xs) {
  size_t out = 0;
  for (; !isnil(xs); xs = cdr(xs)) { out++; }
  return out;
}


lobj_t ** list_to_vector(lobj_t * xs) {
  size_t idx = list_len(xs);
  lobj_t ** out = malloc(sizeof(lobj_t*) * (idx+1));

  for (size_t i = 0; i < idx; i++) {
    out[i] = car(xs);
    xs = cdr(xs);
  }

  out[idx] = NIL;

  return out;
}

#endif

