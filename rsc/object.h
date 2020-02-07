/*

Types

Types work similar to Python.

All types must begin with the LOBJ_HEAD macro,
which adds a type code to the object. Objects are
not passed around directly; rather, they are cast to
pointers to lobj_t, which is the generic base type.
When the object's data needs tobe recovered, they
are cast to the appropriate type based on the type code. 

This is the basic system I want, but many future refinements
will be needed.

*/
#ifndef object_h
#define object_h
#include "rascal.h"

// type codes
enum { LOBJ_CONS, LOBJ_SEXPR, LOBJ_SYM, LOBJ_ENV, LOBJ_ERR, LOBJ_PROC, LOBJ_NUM };

/*
GC tags. GC_GREY is included for use in a future implementation
of a tricolor collector. GC_WHITE objects will be collected when
the garbage collector is run. GC_BLACK objects are protected from
collection.
 */
enum { GC_WHITE, GC_GREY, GC_BLACK };
 
/*
  The generic LOBJ_HEAD macro adds type and GC information to the front of every object
 */
#define LOBJ_HEAD \
  int type;       \
  int tag;        \
  int quote;      \
  lobj_t * next;

typedef struct _lobj_t {
  LOBJ_HEAD
} lobj_t;

#define LOBJ_CAST(obj) ((lobj_t*)(obj))

typedef struct _num_t {
  LOBJ_HEAD
  long value;
} num_t;

typedef struct _err_t{
  LOBJ_HEAD
  char * msg;
} err_t;

typedef struct _cons_t {
  LOBJ_HEAD
  lobj_t * _car;
  lobj_t * _cdr;
} cons_t;

typedef struct _sym_t {
  LOBJ_HEAD
  char * name;
} sym_t;

typedef struct _env_t {
  LOBJ_HEAD
  char * name;
  lobj_t * binding;
  struct _env_t * left;
  struct _env_t * right;
} env_t;

/*

Procedures

For simplicity, the types are defined to take pointers
to arrays of lobj_t pointers (so that separate types don't have to
be defined for functions with different aritys).

*/

typedef struct _lambda_t {
  LOBJ_HEAD
  int arity;
  env_t * env;
  proc_t body;
    } lambda_t;

// Type/nil checking macros
#define iscons(obj)  ((obj)->type == LOBJ_CONS)
#define issexpr(obj) ((obj)->type == LOBJ_SEXPR)
#define isnum(obj)   ((obj)->type == LOBJ_NUM)
#define issym(obj)   ((obj)->type == LOBJ_SYM)
#define isenv(obj)   ((obj)->type == LOBJ_ENV)
#define iserr(obj)   ((obj)->type == LOBJ_ERR)
#define isproc(obj)  ((obj)->type == LOBJ_PROC)
#define isnil(obj)   ((uint64_t)(obj)==(uint64_t)NIL)

/* Forward declarations */
// Type constructors
err_t * mk_err(char *, ...);
lobj_t * new_err(char *, ...);
num_t * mk_num(long);
lobj_t * new_num(long);
cons_t * mk_cons(lobj_t *, lobj_t *);
lobj_t * new_cons(lobj_t *, lobj_t *);
sym_t * mk_sym(char *);
lobj_t * new_sym(char *);
env_t * mk_env(char *, lobj_t *);
lobj_t * new_env(char *, lobj_t *);
lambda_t * mk_proc(proc_t, env_t *, int);
lobj_t * new_proc(proc_t, env_t *, int);

// Safecast operators
cons_t * tocons(lobj_t *);
num_t * tonum(lobj_t *);
err_t * toerr(lobj_t *);
sym_t * tosym(lobj_t *);
env_t * toenv(lobj_t *);
lambda_t * toproc(lobj_t *);
cons_t * tosexpr(lobj_t *);

// Helpers & primitives
lobj_t * lobj_copy(lobj_t *);
lobj_t * lobj_quote_copy(lobj_t *);
lobj_t * unquote(lobj_t *);
lobj_t * lookup(char *, env_t *, lobj_t *);
void intern(sym_t *, env_t *, lobj_t *);
lobj_t * prim_add(lobj_t * args[2]);
lobj_t * prim_sub(lobj_t * args[2]);
lobj_t * prim_mul(lobj_t * args[2]);
lobj_t * prim_div(lobj_t * args[2]);
lobj_t * prim_cons(lobj_t * args[2]);
lobj_t * prim_head(lobj_t * args[1]);
lobj_t * prim_tail(lobj_t * args[1]);
lobj_t * prim_def(lobj_t * args[3]);
lobj_t * prim_eval(lobj_t * args[2]);
lobj_t * prim_apply(lobj_t * args[2]);
lobj_t * prim_globals(lobj_t ** args);
lobj_t * prim_allocations(lobj_t ** args);

/* Macros  */
#define LASSERT(cond, fmt, ...)                     \
  if (!(cond)) { ROOT = NIL;                        \
      CURRENT_ERROR = new_err(fmt, ##__VA_ARGS__);  \
      longjmp(TOPLEVEL, 1); }

 
// Accessors and mutators for cons. Those prefixed with f are unsafe but faster
#define car(pair)            (tocons(pair)->_car)
#define cdr(pair)            (tocons(pair)->_cdr)
#define cars(pair)           (tosexpr(pair)->_car)
#define cdrs(pair)           (tosexpr(pair)->_cdr)
#define setcar(pair, value)  (tocons(pair)->_car = (value))
#define setcdr(pair, value)  (tocons(pair)->_cdr = (value))
#define setcars(pair, value) (tosexpr(pair)->_car = (value))
#define setcdrs(pair, value) (tosexpr(pair)->_cdr = (value))
#define fcar(pair)           (((cons_t*)(pair))->_car)
#define fcdr(pair)           (((cons_t*)(pair))->_cdr)
#define fsetcar(pair, v)     (((cons_t*)(pair))->_car = (v))
#define fsetcdr(pair, v)     (((cons_t*)(pair))->_cdr = (v))

// Macros for comparing symbols and environments. Those prefixed with f are unsafe but faster
#define cmpsym(sym1, sym2)   (strcmp(tosym(sym1)->name, tosym(sym2)->name))
#define cmpstrsym(str, sym)  (strcmp((str), tosym(sym)->name))
#define cmpsymstr(sym, str)  (strcmp(tosym(sym)->name, (str)))
#define cmpenv(env1, env2)   (strcmp(toenv(env1)->name, toenv(env2)->name))
#define cmpstrenv(str, env)  (strcmp((str), toenv(env)->name))
#define cmpenvstr(env, str)  (strcmp(toenv(env)->name, (str)))
#define cmpsymenv(sym, env)  (strcmp(tosym(sym)->name, toenv(env)->name))
#define cmpenvsym(env, sym)  (strcmp(toenv(env)->name, tosym(sym)->name))
#define fcmpsym(sym1, sym2)  (strcmp((sym1)->name, (sym2)->name))
#define fcmpstrsym(str, sym) (strcmp((str), (sym)->name))
#define fcmpsymstr(sym, str) (strcmp((sym)->name, (str)))
#define fcmpenv(env1, env2)  (strcmp((env1)->name, (env2)->name))
#define fcmpstrenv(str, env) (strcmp((str), (env)->name))
#define fcmpenvstr(env, str) (strcmp((env)->name, (str)))
#define fcmpsymenv(sym, env) (strcmp((sym)->name, (env)->name))
#define fcmpenvsym(env, sym) (strcmp((env)->name, (sym)->name))

#endif
