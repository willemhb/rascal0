#include "eval.h"


lobj_t * bind_args(lambda_t * fun, lobj_t * args) {
  lobj_t * formals = fun->formals;
  lobj_t * bound_env = *(fun->env);

  // Bind formals in the environment of the procedure body
  for (; !isnil(args); args = cdr(args)) {
    bound_env = intern(car(formals), &bound_env, car(args));
    formals = cdr(formals);
  }

  LASSERT(isnil(formals), "arity error")
  return bound_env;
}



lobj_t * lobj_eval(lobj_t * v, lobj_t ** env) {
  lobj_t * out = v;

 switch (out->type) {
  case LOBJ_NUM:
  case LOBJ_ERR:
  case LOBJ_PROC:
  case LOBJ_PRIM:
  case LOBJ_STR:
    break;
  case LOBJ_SYM:{
    out = lookup(out, env);
    break;
  }
  case LOBJ_CONS:{
    // Call recursively on car and cdr.
     lobj_t * head = lobj_eval(car(out), env);
     lobj_t * tail = cdr(out);

     out = (isproc(head) || isprim(head)) ?
           apply(head, env, tail) :
           new_cons(head, lobj_eval(tail, env));
     break;
  }
 }  

 return out;
}


lobj_t * lobj_expand(lobj_t * v, lobj_t ** env) {
  lobj_t * out = v, * binding;

 switch (out->type) {
  case LOBJ_NUM:
  case LOBJ_ERR:
  case LOBJ_PROC:
  case LOBJ_PRIM:
  case LOBJ_STR:
    break;
  // Symbols should only be substituted if they represent macros
  case LOBJ_SYM:{
    binding = lookup(out, env);
    out = ismacro(binding) ? binding : out;
    break;
  }
  case LOBJ_CONS:{
    // Call recursively on car and cdr.
     lobj_t * head = lobj_expand(car(out), env);
     lobj_t * tail = lobj_expand(cdr(out), env);

     out = ismacro(head) ?
           apply(head, env, tail) :
           new_cons(head, tail);
     break;
  }
 }  
 return out;
}


lobj_t * apply(lobj_t * fun, lobj_t ** env, lobj_t * args) {
  switch (fun->type) {
  case LOBJ_PRIM:{
    prim_t * body = toprim(fun);
    args = (body->evaltype) == EVAL_PROC ? lobj_eval(args, env) : args;
    return apply_prim(fun, env, args);
  }case LOBJ_PROC:{
     lambda_t * lmbody = toproc(fun);
     args = (lmbody->evaltype) == EVAL_PROC ? lobj_eval(args, env) : args;
     return apply_lambda(fun, args);
  }default: return new_err("Type Error: expected type function, got %i", fun->type);
    }
}

lobj_t * apply_prim(lobj_t * fun, lobj_t ** env, lobj_t * args) {
  prim_t * pfun = toprim(fun);
  lobj_t ** argstup = getargs(fun, args);

  return (pfun->body(argstup, env));
}

lobj_t * apply_lambda(lobj_t * fun, lobj_t * args) {
  lambda_t * lfun = toproc(fun);
  lobj_t * bound_env = bind_args(lfun, args);

  return lobj_eval(lfun->body, &bound_env);
}
