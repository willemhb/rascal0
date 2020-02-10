#include "eval.h"


lobj_t * lobj_eval(lobj_t * v, sym_t * env) {
  lobj_t * out = v;
  // Quoted items are self evaluating
  if (out->quote) return lobj_qeval(out, env);
  switch (out->type) {
  case LOBJ_NUM:
  case LOBJ_ERR:
  case LOBJ_PROC:
  case LOBJ_PRIM:
    break;
  case LOBJ_SYM:{
    sym_t * sym_name = tosym(out);
    out = lookup(sym_name, env, NULL);
    LASSERT(!isunbound(out), "Unbound symbol %s", sym_name->name)
    break;
  }case LOBJ_CONS:{
    // Call recursively on car and cdr.
     lobj_t * head = lobj_eval(car(out), env);
     lobj_t * tail  = cdr(out);
     out = apply(head, env, tail);
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
  case LOBJ_PRIM:
  case LOBJ_PROC:
  case LOBJ_SYM:
    break;
  case LOBJ_CONS:{
    // Call recursively on car and cdr.
     lobj_t * head = lobj_eval(car(out), env);
     lobj_t * tail = lobj_eval(cdr(out), env);
     out = new_cons(head, tail);
     out->quote = 1;
     break;
   }
  }
  
  return out;
}

lobj_t * apply(lobj_t * fun, sym_t * env, lobj_t * args) {
  switch (fun->type) {
  case LOBJ_PRIM: return apply_prim(toprim(fun), env, args);
  case LOBJ_PROC:{
    return apply_lambda(toproc(fun), args);
  }
  }

  return new_err("Type Error: expected type function, got %i", fun->type);
}

lobj_t * apply_lambda(lambda_t * fun, lobj_t * args) {
  lobj_t * formals = fun->formals;
  // Bind formals in the environment of the procedure body
  for (; !isnil(args); args = cdr(args)) {
    update(tosym(car(fun->formals)), fun->env, car(args));
    formals = cdr(formals);
  }

  lobj_t * out = NIL;
  
  for (lobj_t * body = fun->body ; !isnil(body); body = cdr(body)) {
    out = lobj_eval(unquote(body), fun->env);
  }

  return out;
}

lobj_t * apply_prim(prim_t * fun, sym_t * env, lobj_t * args) {
  lobj_t * argstuple[fun->arity];
  int i = 0;

  while (!isnil(args)) {
    LASSERT(i < fun->arity, "Too many args, exceeded %i", fun->arity)
      argstuple[i] = lobj_eval(unquote(car(args)), env);
    args = cdr(args);
    i++;
  }

  LASSERT(i == fun->arity, "Expected %i args, got %i", fun->arity, i)

  return fun->body(argstuple);
}
