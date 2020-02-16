#include "eval.h"
#include "printer.h"

lobj_t * lobj_eval(lobj_t * v, lobj_t ** env) {
  lobj_t * out = v;

 switch (out->type) {
  case LOBJ_FORM:
  case LOBJ_NUM:
  case LOBJ_ERR:
  case LOBJ_PROC:
  case LOBJ_PRIM:
  case LOBJ_STR:
    break;
  case LOBJ_SYM: {
    out = lookup(out, env);
    break;
  }
  case LOBJ_CONS:{
    // Call recursively on car and cdr.
     lobj_t * head = lobj_eval(car(out), env);
     lobj_t * tail = cdr(out);

     if (isproc(head) || isprim(head)) {
       out = apply(head, env, tail);
     } else {
       out = new_cons(head, lobj_eval(tail, env));
     }
     break;
   }
  }
  
  return out;
}


lobj_t * apply(lobj_t * fun, lobj_t ** env, lobj_t * args) {
  switch (fun->type) {
  case LOBJ_PRIM: return apply_prim(toprim(fun), env, args);
  case LOBJ_PROC: return apply_lambda(toproc(fun), args);
  default: return new_err("Type Error: expected type function, got %i", fun->type);
    }
}


lobj_t * apply_lambda(lambda_t * fun, lobj_t * args) {
  lobj_t * formals = fun->formals;
  // Bind formals in the environment of the procedure body
  for (; !isnil(args); args = cdr(args)) {
    update(car(formals), &(fun->env), car(args));
    formals = cdr(formals);
  }

  lobj_t * out = lobj_eval(fun->body, &fun->env);

  return out;
}


lobj_t * apply_prim(prim_t * fun, lobj_t ** env, lobj_t * args) {
  lobj_t * argstuple[fun->argc];
  int i = 0;

  while (!isnil(args)) {
    LASSERT(i < fun->argc, "Too many args, exceeded %i", fun->argc)
      argstuple[i] = car(args);
    args = cdr(args);
    i++;
  }

  LASSERT(i == fun->argc, "Expected %i args, got %i", fun->argc, i)

    return (fun->body(argstuple, env));
}
