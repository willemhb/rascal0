#include "eval.h"


lobj_t * lobj_eval(lobj_t * v, env_t * env) {
  lobj_t * out = v;
  switch (out->type) {
  case LOBJ_NUM:
  case LOBJ_ERR:
  case LOBJ_ENV:
  case LOBJ_PROC:
    break;
  case LOBJ_SYM:{
    out = lookup(tosym(out)->name, env, out);
    break;
  }case LOBJ_CONS:{
    // Call recursively on car and cdr.
     setcar(out, lobj_eval(car(out), env));
     setcdr(out, lobj_eval(cdr(out), env));
     break;
   }case LOBJ_SEXPR:{
     lobj_t * head = lobj_eval(cars(out), env);
    out = apply(toproc(head), lobj_eval(cdrs(out), env));
    break;
   }
  }
  
  return out;
}

lobj_t * apply(lambda_t * fun, lobj_t * args) {
  lobj_t * argstuple[fun->arity];
  int i = 0;

  while (!isnil(args)) {
    LASSERT(i < fun->arity, "Expected %i args, got %i", fun->arity, i)
    argstuple[i] = car(args);
    args = cdr(args);
    i++;
  }

  LASSERT(i == fun->arity, "Expected %i args, got %i", fun->arity, i)

  return fun->body(argstuple);
}
