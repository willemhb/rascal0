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
  s->binding = binding ? binding : LOBJ_CAST(s);
  s->left = s->right = s->parent = tosym(NIL);

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
  v->quote = 0;

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


prim_t * mk_prim(proc_t body, int arity) {
  prim_t * fun = malloc(sizeof(prim_t));
  fun->type = LOBJ_PRIM;
  fun->tag = GC_WHITE;
  fun->arity = arity;
  fun->body = body;

  return fun;
}


lobj_t * new_prim(proc_t body, int arity) {
  lobj_t * fun = LOBJ_CAST(mk_prim(body, arity));
  LINK(fun);

  return fun;
}


lambda_t * mk_proc(lobj_t * formals, lobj_t * body, sym_t * parent) {
  lambda_t * fun = malloc(sizeof(lambda_t));
  fun->type = LOBJ_PROC;
  fun->tag = GC_WHITE;
  fun->formals = formals;
  fun->body = body;
  sym_t * locals = NULL;

  // Create bindings for formals in env. These are bound every time the function
  // Is called.
  for (; !isnil(formals); formals = cdr(formals)) {
    intern(tosym(car(formals)), locals, NULL);
  }

  fun->env = locals;
  fun->env->parent = parent;
  
  LINK(LOBJ_CAST(locals));

  fun->quote = 0;

  return fun;
}

lobj_t * new_proc(lobj_t * formals, lobj_t * body, sym_t * parent) {
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
       lambda_t * out_proc = mk_proc(proc->formals, proc->body, proc->env->parent);
       out = LOBJ_CAST(out_proc);
       break;
     }case LOBJ_PRIM: return obj;
      case LOBJ_CONS: return new_cons(lobj_copy(car(obj)), lobj_copy(cdr(obj)));
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

sym_t ** assoc(char * s, sym_t ** env) {
  sym_t ** curr = env;
  int cmp;

 while (*curr && (cmp = fcmpstrsym(s, *curr))) {
    if (cmp < 0) {
      curr = &(*curr)->left;
    } else {
      curr = &(*curr)->right;
    }
  }

  return curr;  
}


void intern(sym_t * new, sym_t * env, lobj_t * value) {
  new->binding = value;
  sym_t * prev, * curr = env;
  int cmp, isleft;

  while (curr && (cmp = fcmpsym(new, curr))) {
    prev = curr;
    if (cmp < 0) {
      isleft = 1;
      prev = curr;
      curr = curr->left;
    } else if (cmp > 0) {
      isleft = 0;
      prev = curr;
      curr = curr->right;
    } else return;
  }

  if (isleft) prev->left = new;
  else prev->right = new;
}

void update(sym_t * new, sym_t * env, lobj_t * value) {
  sym_t ** table = &env;
  sym_t ** result = assoc(new->name, table);

  if (isnil(*result)) return;

  (*result)->binding = value;
  
}

lobj_t * lookup(sym_t * s, sym_t * env, lobj_t * defaultval) {
  sym_t * curr = env;
  int cmp = 1;

  while (curr && (cmp = fcmpsym(s, curr))) {
    if (cmp < 0) {
      curr = curr->left;
    } else {
      curr = curr->right;
    }
  }

  if (curr == NULL) {
    if (env->parent == NULL) return (defaultval ? defaultval : UNBOUND);
    // Try to find the variable in the parent environment
    return lookup(s, env->parent, defaultval);
  } else { return curr->binding; }
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

lobj_t * prim_mod(lobj_t * args[2]) {
  num_t * x = tonum(args[0]);
  num_t * y = tonum(args[1]);
  LASSERT(y->value != 0, "Modulo by Zero Error.")

  return new_num(x->value % y->value);
}

lobj_t * prim_cons(lobj_t * args[2]) {
  lobj_t * out = new_cons(args[0], args[1]);
  out->quote = 1;

  return out;
}

lobj_t * prim_head(lobj_t * args[1]) {
  return car(args[0]);
}

lobj_t * prim_tail(lobj_t * args[1]) {
  return cdr(args[0]);
}

lobj_t * prim_def(lobj_t * args[3]) {
  sym_t * name = tosym(args[0]);
  // Unquote symbol for table insertion
  name->quote = 0;
  lobj_t * value = args[1];
  sym_t * env = tosym(args[2]);

  intern(name, env, value);
  return value;
}

lobj_t * prim_setq(lobj_t * args[3]) {
  sym_t * name = tosym(args[0]);
  // Unquote symbol for table insertion
  name->quote = 0;
  lobj_t * value = args[1];
  sym_t * env = tosym(args[2]);

  sym_t * result = *assoc(name->name, &env);
  LASSERT(!isunbound(result), "Unbound symbol %s\n", name->name)
  result->binding = value;
  return value;
}


lobj_t * prim_eval(lobj_t * args[2]) {
  return lobj_eval(args[0], tosym(args[1]));
}

lobj_t * prim_apply(lobj_t * args[3]) {
  return apply(args[0], tosym(args[1]), args[2]);
}

lobj_t * prim_globals(lobj_t ** args) {
  return LOBJ_CAST(GLOBALS);
}

// For debugging from the REPL
lobj_t * prim_allocations(lobj_t ** args) {
  printf("%i Allocations\n", ALLOCATIONS);
  return NIL;
}


lobj_t * prim_fn(lobj_t * args[3]) {
  return new_proc(args[0], args[1], tosym(args[2]));
}
