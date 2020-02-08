#include "eval.h"


lobj_t * lobj_eval(lobj_t * v, sym_t * env) {
  lobj_t * out = v;
  // Quoted items are self evaluating
  if (out->quote) return lobj_qeval(out, env);
  switch (out->type) {
  case LOBJ_NUM:
  case LOBJ_ERR:
  case LOBJ_PROC:
    break;
  case LOBJ_SYM:{
    out = lookup(tosym(out)->name, env, out);
    break;
  }case LOBJ_CONS:{
    // Call recursively on car and cdr.
     lobj_t * head = lobj_eval(car(out), env);
     lobj_t * tail = lobj_eval(cdr(out), env);
     out = apply(toproc(head), tail);
     break;
   }
  }
  
  return out;
}


lobj_t * lobj_qeval(lobj_t * v, sym_t * env) {
  lobj_t * out = v;
  switch (out->type) {
  case LOBJ_NUM:
  case LOBJ_ERR:
  case LOBJ_PROC:
  case LOBJ_SYM:
    break;
  case LOBJ_CONS:{
    // Call recursively on car and cdr.
     lobj_t * head = lobj_eval(car(out), env);
     lobj_t * tail = lobj_eval(cdr(out), env);
     out = new_cons(head, tail);
     break;
   }
  }
  
  return out;
};

lobj_t * apply(lambda_t * fun, lobj_t * args) {
  lobj_t * argstuple[fun->arity];
  int i = 0;

  while (!isnil(args)) {
    LASSERT(i < fun->arity, "Too many args, exceeded %i", fun->arity)
    argstuple[i] = car(args);
    args = cdr(args);
    i++;
  }

  LASSERT(i == fun->arity, "Expected %i args, got %i", fun->arity, i)

  return fun->body(argstuple);
}
