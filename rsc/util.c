#include "util.h"


/*
Utilities for working on the backend.
 */

tuple_t * new_tuple(int len) {
  tuple_t * out = malloc(sizeof(tuple_t));
  out->values = malloc(sizeof(lobj_t*) * len);
  out->len = len;

  return out;
}

void del_tuple(tuple_t * garbage) {
  free(garbage->values);
  free(garbage);

  return;
}

// Get the length of a lisp list as a c size_t
int list_len(lobj_t * xs) {
  int out = 0;
  for (; !isnil(xs); xs = cdr(xs)) { out++; }
  return out;
}


tuple_t * list_to_vector(lobj_t * xs) {
  int idx = list_len(xs);
  tuple_t * out = new_tuple(idx);

  for (int i = 0; i < idx; i++) {
    out->values[i] = car(xs);
    xs = cdr(xs);
  }

  return out;
}

// Get a vector of arguments and check arity
lobj_t ** getargs(lobj_t * fun, lobj_t * args) {
  lobj_t ** out;
  tuple_t * argstup = list_to_vector(args);
  int len = argstup->len;
  int argc = isprim(fun) ? toprim(fun)->argc : toproc(fun)->argc;
  int vararg = isprim(fun) ? toprim(fun)->vararg : toproc(fun)->vararg;

  if (vararg) {
    if (((argstup->len) % (argc - 1)) != 1) {
      del_tuple(argstup);
      LRAISE("arity error: expected %d args to #, got %d", argc, len);
    }
  } else if (argc != len) {
    del_tuple(argstup);
    LRAISE("arity error: expected %d args to #, got %d", argc, len);
  }

  out = argstup->values;
  free(argstup);

  return out;
  }
