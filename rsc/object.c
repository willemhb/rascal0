#include "object.h"
#include "eval.h"

cons_t * mk_cons(lobj_t * car_, lobj_t * cdr_) {
  cons_t * v = malloc(sizeof(cons_t));
  v->type = LOBJ_CONS;
  v->tag = GC_WHITE;
  v->quote = 0;
  v->_car = car_;
  v->_cdr = cdr_;

  return v;
}

lobj_t * new_cons(lobj_t * car_, lobj_t * cdr_) {
  lobj_t * out = LOBJ_CAST(mk_cons(car_, cdr_));
  LINK(out);

  return out;
}

sym_t * mk_sym(char * name, lobj_t * binding) {
  sym_t * s = malloc(sizeof(sym_t));
  s->type = LOBJ_SYM;
  s->tag = GC_WHITE;
  s->quote = 0;
  str_init(s->name, name);
  s->binding = binding ? binding : UNBOUND;

  return s;
}

lobj_t * new_sym(char * name, lobj_t * binding) {
  lobj_t * out = LOBJ_CAST(mk_sym(name, binding));
  LINK(out);

  return out;
}

num_t * mk_num(long value) {
  num_t * n = malloc(sizeof(num_t));
  n->tag = GC_WHITE;
  n->type = LOBJ_NUM;
  n->quote = 0;
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
  v->quote = 0;

  va_list va;
  va_start(va, fmt);
  char * msg_buff = malloc(512);
  vsnprintf(msg_buff, 511, fmt, va);
  v->msg = malloc(strlen(msg_buff)+1);
  v->msg = strcpy(v->msg, msg_buff);
  free(msg_buff);

  va_end(va);
  return v;
}

lobj_t * new_err(char * fmt, ...) {
  err_t * v = malloc(sizeof(err_t));
  v->tag = GC_WHITE;
  v->type = LOBJ_ERR;
  v->quote = 0;

  va_list va;
  va_start(va, fmt);
  char * msg_buff = malloc(512);
  vsnprintf(msg_buff, 511, fmt, va);
  v->msg = malloc(strlen(msg_buff)+1);
  v->msg = strcpy(v->msg, msg_buff);
  free(msg_buff);

  va_end(va);

  lobj_t * out = LOBJ_CAST(v);
  LINK(out);
  return out;
}


lambda_t * mk_proc(proc_t body, sym_t * env, int arity) {
  lambda_t * fun = malloc(sizeof(lambda_t));
  fun->type = LOBJ_PROC;
  fun->tag = GC_WHITE;
  fun->body = body;
  fun->env = env;
  fun->arity = arity;
  fun->quote = 0;

  return fun;
}

lobj_t * new_proc(proc_t body, sym_t * env, int arity) {
  lobj_t * out = LOBJ_CAST(mk_proc(body, env, arity));
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
SAFECAST_OP(cons_t*, sexpr, "sexpr")

// Deep copy operation
// Carefully consider whether copying an object means copying an environment!
  lobj_t * lobj_copy(lobj_t * obj) {
  LASSERT(obj != NULL, "Attempt to access illegal memory.")
  lobj_t * out;

  if (obj->quote) return lobj_quote_copy(obj);
    
    switch (obj->type) {
    case LOBJ_NUM: return new_num(tonum(obj)->value);
    case LOBJ_ERR: return new_err(toerr(obj)->msg);
    case LOBJ_SYM:{
      sym_t * env = tosym(obj);
      sym_t * out_env = mk_sym(env->name, lobj_copy(env->binding));
      out_env->left = tosym(lobj_copy(LOBJ_CAST(env->left)));
      out_env->right = tosym(lobj_copy(LOBJ_CAST(env->right)));
      out = LOBJ_CAST(out_env);
      break;
    }case LOBJ_PROC:{
       lambda_t * proc = toproc(obj);
       lambda_t * out_proc = mk_proc(proc->body, tosym(lobj_copy(LOBJ_CAST(proc->env))), proc->arity);
       out = LOBJ_CAST(out_proc);
       break;
     }case LOBJ_CONS: return new_cons(lobj_copy(car(obj)), lobj_copy(cdr(obj)));
     }

    LINK(out);
    return out;
    }

/* NB this implementation of quoting will break in a multithreaded environment  */
lobj_t * lobj_quote_copy(lobj_t * obj) {
  obj->quote = 0;
  lobj_t * out = lobj_copy(obj);
  obj->quote =1;
  out->quote = 1;

  return out;
}

// Return an unquoted copy of obj
lobj_t * unquote(lobj_t * obj) {
  obj->quote = 0;
  lobj_t * out = lobj_copy(obj);
  obj->quote = 1;

  return out;
}

// Helpers for working with symbols.
void intern(sym_t * new, sym_t * env, lobj_t * binding) {
  sym_t * prev, * curr = env;
  int cmp, isleft = 0;

  while (curr) {
    cmp = fcmpsym(new, curr);
    if (cmp < 0) {
      prev = curr;
      curr = curr->left;
      isleft = 1;
    } else if (cmp > 0) {
      prev = curr;
      curr = curr->right;
      isleft = 0;
    } else {
      return;
    }
  }

  if (isleft) {
    prev->left = mk_sym(new->name, binding);
    LINK(LOBJ_CAST(prev->left));
  } else {
    prev->right = mk_sym(new->name, binding);
    LINK(LOBJ_CAST(prev->right));
  }
}

lobj_t * lookup(char * s, sym_t * env, lobj_t * defaultval) {
  sym_t * curr = env;
  int cmp;

  while (curr && (cmp = fcmpstrsym(s, curr))) {
    if (cmp < 0) {
      curr = curr->left;
    } else {
      curr = curr->right;
    }
  }

  if (curr == NULL) {
    return defaultval ? defaultval : UNBOUND;
  }

  return curr->binding;
}

// Primitive operations and functions
lobj_t * prim_add(lobj_t * args[2]) {
  num_t * x = tonum(args[0]);
  num_t * y = tonum(args[1]);

  return new_num(x->value + y->value);
}

lobj_t * prim_sub(lobj_t * args[2]) {
  num_t * x = tonum(args[0]);
  num_t * y = tonum(args[1]);

  return new_num(x->value - y->value);
}

lobj_t * prim_mul(lobj_t * args[2]) {
  num_t * x = tonum(args[0]);
  num_t * y = tonum(args[1]);

  return new_num(x->value * y->value);
}

lobj_t * prim_div(lobj_t * args[2]) {
  num_t * x = tonum(args[0]);
  num_t * y = tonum(args[1]);
  LASSERT(y->value != 0, "Divide by Zero Error.")

  return new_num(x->value / y->value);
}

lobj_t * prim_cons(lobj_t * args[2]) {
  return new_cons(args[0], args[1]);
}

lobj_t * prim_head(lobj_t * args[1]) {
  return car(args[0]);
}

lobj_t * prim_tail(lobj_t * args[1]) {
  return cdr(args[0]);
}

lobj_t * prim_def(lobj_t * args[3]) {
  sym_t * name = tosym(unquote(args[0]));
  lobj_t * value = args[1];
  sym_t * env = tosym(args[2]);

  intern(name, env, value);
  return value;
}

lobj_t * prim_eval(lobj_t * args[2]) {
  return lobj_eval(args[0], tosym(args[1]));
}

lobj_t * prim_apply(lobj_t * args[2]) {
  return apply(toproc(args[0]), args[1]);
}

lobj_t * prim_globals(lobj_t ** args) {
  return LOBJ_CAST(GLOBALS);
}

// For debugging from the REPL
lobj_t * prim_allocations(lobj_t ** args) {
  printf("%i Allocations\n", ALLOCATIONS);
  return NIL;
}
