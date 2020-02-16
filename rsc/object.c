#include "object.h"
#include "eval.h"
#include "printer.h"

cons_t * mk_cons(lobj_t * car_, lobj_t * cdr_) {
  cons_t * v = malloc(sizeof(cons_t));
  v->type = LOBJ_CONS;
  v->tag = GC_WHITE;
  v->_car = car_;
  v->_cdr = cdr_;

  return v;
}

lobj_t * new_cons(lobj_t * car_, lobj_t * cdr_) {
  lobj_t * out = LOBJ_CAST(mk_cons(car_, cdr_));
  LINK(out);

  return out;
}

sym_t * mk_sym(char * name) {
  sym_t * s = malloc(sizeof(sym_t));
  s->type = LOBJ_SYM;
  s->tag = GC_WHITE;
  str_init(s->name, name);

  return s;
}

lobj_t * new_sym(char * name) {
  lobj_t * out = LOBJ_CAST(mk_sym(name));
 LINK(out);

  return out;
}

num_t * mk_num(long value) {
  num_t * n = malloc(sizeof(num_t));
  n->tag = GC_WHITE;
  n->type = LOBJ_NUM;
  n->value = value;

  return n;
}

lobj_t * new_num(long value) {
    lobj_t * out = LOBJ_CAST(mk_num(value));
  LINK(out);

  return out;
}

err_t * mk_err(char * fmt, ...) {
  err_t * v = malloc(sizeof(err_t));
  v->tag = GC_WHITE;
  v->type = LOBJ_ERR;

  va_list va;
  va_start(va, fmt);
  v-> msg = malloc(512);
  vsnprintf(v->msg, 511, fmt, va);
  v->msg = realloc(v->msg, strlen(v->msg)+1);
  va_end(va);

  return v;
}

lobj_t * new_err(char * fmt, ...) {
  err_t * v = malloc(sizeof(err_t));
  v->tag = GC_WHITE;
  v->type = LOBJ_ERR;

  va_list va;
  va_start(va, fmt);
  v-> msg = malloc(512);
  vsnprintf(v->msg, 511, fmt, va);
  v->msg = realloc(v->msg, strlen(v->msg)+1);
  va_end(va);

  lobj_t * out = LOBJ_CAST(v);
  LINK(out);
  return out;
}

prim_t * mk_prim(proc_t body, int argc) {
  prim_t * fun = malloc(sizeof(prim_t));
  fun->type = LOBJ_PRIM;
  fun->tag = GC_WHITE;
  fun->argc = argc;
  fun->body = body;

  return fun;
}

lobj_t * new_prim(proc_t body, int argc) {
  lobj_t * fun = LOBJ_CAST(mk_prim(body, argc));
  LINK(fun);

  return fun;
}

lambda_t * mk_proc(lobj_t * formals, lobj_t * body, lobj_t * parent) {
  lambda_t * fun = malloc(sizeof(lambda_t));
  fun->type = LOBJ_PROC;
  fun->tag = GC_WHITE;
  fun->formals = formals;
  fun->body = body;
  lobj_t * locals = parent;

  // Create bindings for formals in env. These are bound every time the function
  // Is called.
  for (; !isnil(formals); formals = cdr(formals)) {
    locals = intern(car(formals), &locals, UNBOUND);
  }
  fun->env = locals;

  return fun;
}

lobj_t * new_proc(lobj_t * formals, lobj_t * body, lobj_t * parent) {
  lobj_t * out = LOBJ_CAST(mk_proc(formals, body, parent));
  LINK(out);

  return out;
}


// Safecast macro (credit Jeff Bezanson, author of FemtoLisp)
#define SAFECAST_OP(ctype,ltype,name)				     \
  ctype to##ltype(lobj_t * v)                                        \
  {                                                                  \
    LASSERT(is##ltype(v), "Expected type %s, got %d", name, v->type) \
    return (ctype)v;                                                 \
  }

SAFECAST_OP(cons_t*, cons, "cons")
SAFECAST_OP(num_t*, num, "num")
SAFECAST_OP(err_t*, err, "err")
SAFECAST_OP(sym_t*, sym, "sym")
SAFECAST_OP(lambda_t*, proc, "proc")
SAFECAST_OP(prim_t*, prim, "prim")

// Deep copy operation
// Carefully consider whether copying an object means copying an environment!
  lobj_t * lobj_copy(lobj_t * obj) {
  LASSERT(obj != NULL, "Attempt to access illegal memory.")
  lobj_t * out;
    
    switch (obj->type) {
    case LOBJ_NUM: return new_num(tonum(obj)->value);
    case LOBJ_ERR: return new_err(toerr(obj)->msg);
    case LOBJ_SYM: return new_sym(tosym(obj)->name);
    case LOBJ_PROC:{
       lambda_t * proc = toproc(obj);
       lambda_t * out_proc = mk_proc(lobj_copy(proc->formals), lobj_copy(proc->body), proc->env);
       out = LOBJ_CAST(out_proc);
       break;
     }case LOBJ_PRIM: return obj;
      case LOBJ_CONS: return new_cons(lobj_copy(car(obj)), lobj_copy(cdr(obj)));
    } 

    return out;
}

// Helpers for working with symbols. For now environments are implemented as sorted lists,
// due to ease of implementation and debugging. This will be upgraded once a stable version
// of the language exists.

lobj_t * assoc(lobj_t * value, lobj_t ** env) {
  if (isnil(env)) return UNBOUND;

  int cmp = cmpsym(value, car(car(*env)));

  if (cmp < 0) return assoc(value, &(cdr(*env)));

  if (cmp > 0) return UNBOUND;
  
  return car(*env);  
}


lobj_t * intern(lobj_t * new, lobj_t ** env, lobj_t * value) {
  if (isnil(env)) return new_cons(new_cons(new, value), *env);

  int cmp = cmpsym(new, car(car(*env)));

  if (cmp < 0) return new_cons(car(*env), intern(new, &(cdr(*env)), value));

  return new_cons(new_cons(new, value), *env);
  
}

void update(lobj_t * key, lobj_t ** env, lobj_t * value) {
  lobj_t * pair = assoc(key, env);

  if (isunbound(pair)) return;

  setcdr(pair, value);
}

lobj_t * lookup(lobj_t * sym, lobj_t ** env) {
  lobj_t * pair = assoc(sym, env);
  if (isunbound(pair)) return UNBOUND;

  return cdr(pair);
}

// Imperatively update environment with new key
void puts_env(lobj_t * key, lobj_t ** env, lobj_t * binding) {
  lobj_t ** prev = NULL, ** curr = env;
  int cmp;
  lobj_t * new_value = new_cons(key, binding);

  if (isnil(*env)) { *env = new_cons(new_value, *env); }

  while (!isnil(*curr)) {
    cmp = cmpsym(key, car(car(*curr)));

    if (cmp >= 0) break;

    prev = curr;
    curr = &cdr(*curr);
    }

  lobj_t * new_env = new_cons(new_value, *curr);

  // Empty environment or new maximal elements special case
  if (prev == NULL) {
    *env = new_env;
    
  } else { setcdr(*prev, new_env);  }
  return;
} 


// Primitive operations and functions
lobj_t * prim_add(lobj_t * args[2], lobj_t ** env) {
  num_t * x = tonum(lobj_eval(args[0], env));
  num_t * y = tonum(lobj_eval(args[1], env));

  return new_num(x->value + y->value);
}

lobj_t * prim_len(lobj_t * list[1], lobj_t ** env) {
  int out = 0;
  lobj_t * xs = lobj_eval(list[0], env);
  for (; !isnil(xs); xs = cdr(xs)) {
    out++;
  }
  
  return new_num(out);
}


lobj_t * prim_eq(lobj_t * args[2], lobj_t ** env) {
  lobj_t * x = lobj_eval(args[0], env);
  lobj_t * y = lobj_eval(args[1], env);
  lobj_t * out = NIL;

  switch (x->type) {
  case LOBJ_NUM:{
    if (isnum(y) && (tonum(x)->value) == (tonum(y)->value)) {
      out = new_sym("t");
    }
    break;
 }case LOBJ_SYM:{
    if (issym(y) && fcmpsym(x,y) == 0) {
      out = new_sym("t");
    }
    break;
  }default: break;
 }

  return out;
}


lobj_t * prim_sub(lobj_t * args[2], lobj_t ** env) {
  num_t * x = tonum(lobj_eval(args[0], env));
  num_t * y = tonum(lobj_eval(args[1], env));

  return new_num(x->value - y->value);
}

lobj_t * prim_mul(lobj_t * args[2], lobj_t ** env) {
  num_t * x = tonum(lobj_eval(args[0], env));
  num_t * y = tonum(lobj_eval(args[1], env));

  return new_num(x->value * y->value);
}

lobj_t * prim_div(lobj_t * args[2], lobj_t ** env) {
  num_t * x = tonum(lobj_eval(args[0], env));
  num_t * y = tonum(lobj_eval(args[1], env));

  LASSERT(y->value != 0, "Divide by Zero Error.")

  return new_num(x->value / y->value);
}

lobj_t * prim_mod(lobj_t * args[2], lobj_t ** env) {
  num_t * x = tonum(lobj_eval(args[0], env));
  num_t * y = tonum(lobj_eval(args[1], env));
  LASSERT(y->value != 0, "Modulo by Zero Error.")

  return new_num(x->value % y->value);
}

lobj_t * prim_cons(lobj_t * args[2], lobj_t ** env) {
  lobj_t * thecar = lobj_eval(args[0], env);
  lobj_t * thecdr = lobj_eval(args[1], env);

  return new_cons(thecar, thecdr);
}

lobj_t * prim_head(lobj_t * args[1], lobj_t ** env) {
  return car(lobj_eval(args[0], env));
}

lobj_t * prim_tail(lobj_t * args[1], lobj_t ** env) {
  return cdr(lobj_eval(args[0], env));
}

lobj_t * prim_eval(lobj_t * args[2], lobj_t ** env) {
  return lobj_eval(args[0], &args[1]);
}

lobj_t * prim_apply(lobj_t * args[3], lobj_t ** env) {
  return apply(lobj_eval(args[0], env), &args[1], args[2]);
}

lobj_t * prim_globals(lobj_t ** args, lobj_t ** env) {
  return LOBJ_CAST(GLOBALS);
}

// For debugging from the REPL
lobj_t * prim_allocations(lobj_t ** args, lobj_t ** env) {
  printf("%i Allocations\n", ALLOCATIONS);
  return NIL;
}

/* lobj_t * prim_print(lobj_t * args[1]) {
  lobj_println(args[0]);
  return NIL;
} */

// Special forms
lobj_t * form_def(lobj_t * args[2], lobj_t ** env) {
  lobj_t * name = args[0];
  lobj_t * binding = lobj_eval(args[1], env);
  puts_env(name, env, binding);

  return binding;
}


lobj_t * form_setq(lobj_t * args[2], lobj_t ** env) {
  lobj_t * name = args[0];
  lobj_t * binding = args[1];
  update(name, env, lobj_eval(binding, env));

  return binding;
}


lobj_t * form_quote(lobj_t * args[1], lobj_t ** env) {
  return args[0];
}


lobj_t * form_if(lobj_t * args[3], lobj_t ** env) {

  if (!isnil(lobj_eval(args[0], env))) {
      return lobj_eval(args[1], env);
    } else {
    return lobj_eval(args[2], env);
  }
}


lobj_t * form_fn(lobj_t * args[2], lobj_t ** env) {
  return new_proc(args[0], args[1], *env);
}


lobj_t * form_do(lobj_t * args[1], lobj_t ** env) {
  lobj_t * out = lobj_eval(args[0], env);

  for (lobj_t * body = args[0]; !isnil(body); body = cdr(body)) {
    out = lobj_eval(car(body), env);
  }

  return out;
}
