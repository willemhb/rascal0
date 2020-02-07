#include "object.h"
#include "eval.h"

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


env_t * mk_env(char * name, lobj_t * binding) {
  if (binding == NULL) {
    binding = NIL;
  }

  env_t * s = malloc(sizeof(env_t));
  s->tag = GC_WHITE;
  s->type = LOBJ_ENV;
  str_init(s->name, name);
  s->binding = binding;
  s->left = s->right = NULL;

  return s;
}

lobj_t * new_env(char * name, lobj_t * binding) {
  lobj_t * out = LOBJ_CAST(mk_env(name, binding));
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


lambda_t * mk_proc(proc_t body, env_t * env, int arity) {
  lambda_t * fun = malloc(sizeof(lambda_t));
  fun->type = LOBJ_PROC;
  fun->tag = GC_WHITE;
  fun->body = body;
  fun->env = env;
  fun->arity = arity;

  return fun;
}

lobj_t * new_proc(proc_t body, env_t * env, int arity) {
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
SAFECAST_OP(env_t*, env, "env")
SAFECAST_OP(lambda_t*, proc, "proc")
SAFECAST_OP(cons_t*, sexpr, "sexpr")


// Helpers for working with symbols.
void intern(sym_t * new, env_t * env, lobj_t * binding) {
  env_t * prev, * curr = env;
  int cmp, isleft = 0;
  if (binding == NULL) binding = NIL;

  while (curr) {
    cmp = fcmpsymenv(new, curr);
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
    prev->left = mk_env(new->name, binding);
    LINK(LOBJ_CAST(prev->left));
  } else {
    prev->right = mk_env(new->name, binding);
    LINK(LOBJ_CAST(prev->right));
  }
}

lobj_t * lookup(char * s, env_t * env, lobj_t * defaultval) {
  env_t * curr = env;
  int cmp;

  while (curr && (cmp = fcmpstrenv(s, curr))) {
    if (cmp < 0) {
      curr = curr->left;
    } else {
      curr = curr->right;
    }
  }

  if (curr == NULL) {
    LASSERT(curr != NULL, "Unbound symbol  %s", s)
    return defaultval;
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
  sym_t * name = tosym(args[0]);
  lobj_t * value = args[1];
  env_t * env = toenv(args[2]);

  intern(name, env, value);
  return value;
}

lobj_t * prim_eval(lobj_t * args[2]) {
  return lobj_eval(args[0], toenv(args[1]));
}

lobj_t * prim_apply(lobj_t * args[2]) {
  return apply(toproc(args[0]), args[1]);
}

lobj_t * prim_globals(lobj_t ** args) {
  return LOBJ_CAST(GLOBALS);
}
