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
enum { LOBJ_CONS, LOBJ_SYM, LOBJ_ERR, LOBJ_PROC, LOBJ_NUM, LOBJ_PRIM, LOBJ_FORM, LOBJ_STR };
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
  // Fields are car and cdr
  lobj_t * _car;
  lobj_t * _cdr;
} cons_t;

typedef struct _sym_t {
  LOBJ_HEAD
  // Fields are binding, parent, left, and right.
  char * name;
} sym_t;

typedef struct _str_t {
  LOBJ_HEAD
  char * value;
} str_t;

/*

Procedures

For simplicity, the types are defined to take pointers
to arrays of lobj_t pointers (so that separate types don't have to
be defined for functions with different aritys). Forms are structurally
identical to primitives but separate types so that the evaluation of their
arguments can be handled in the body of their corresponding procedures
(according to the special rules they abide by).

*/

typedef struct _prim_t {
 LOBJ_HEAD
 int argc;
 proc_t body;
} prim_t;

typedef struct _lambda_t {

  LOBJ_HEAD
  // Fields are formals, env, and body.
  lobj_t * formals;
  lobj_t * body;
  lobj_t * env;
    } lambda_t;

// Type/nil checking macros
#define iscons(obj)    ((obj)->type == LOBJ_CONS)
#define isnum(obj)     ((obj)->type == LOBJ_NUM)
#define issym(obj)     ((obj)->type == LOBJ_SYM)
#define iserr(obj)     ((obj)->type == LOBJ_ERR)
#define isprim(obj)    ((obj)->type == LOBJ_PRIM)
#define isproc(obj)    ((obj)->type == LOBJ_PROC)
#define isstring(obj)  ((obj)->type == LOBJ_STR)
#define isnil(obj)     ((uint64_t)(obj)==(uint64_t)NIL)
#define isunbound(obj) ((uint64_t)(obj)==(uint64_t)UNBOUND)

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
str_t * mk_str(char *);
lobj_t * new_str(char *);
prim_t * mk_prim(proc_t, int);
lobj_t * new_prim(proc_t, int);
lambda_t * mk_proc(lobj_t *, lobj_t *, lobj_t *);
lobj_t * new_proc(lobj_t *, lobj_t *, lobj_t *);

// Safecast operators
cons_t * tocons(lobj_t *);
num_t * tonum(lobj_t *);
err_t * toerr(lobj_t *);
sym_t * tosym(lobj_t *);
str_t * tostring(lobj_t *);
prim_t * toprim(lobj_t *);
lambda_t * toproc(lobj_t *);

// Helpers & primitives
lobj_t * lobj_copy(lobj_t *);
lobj_t * lookup(lobj_t *, lobj_t **);
lobj_t * assoc(lobj_t *, lobj_t **);
lobj_t * intern(lobj_t *, lobj_t **, lobj_t *);
void update(lobj_t *, lobj_t **, lobj_t *);
void puts_env(lobj_t *, lobj_t **, lobj_t *);
lobj_t * prim_len(lobj_t * args[1], lobj_t **);
lobj_t * prim_eq(lobj_t * args[2], lobj_t **);
lobj_t * prim_add(lobj_t * args[2], lobj_t **);
lobj_t * prim_sub(lobj_t * args[2], lobj_t **);
lobj_t * prim_mul(lobj_t * args[2], lobj_t **);
lobj_t * prim_div(lobj_t * args[2], lobj_t **);
lobj_t * prim_mod(lobj_t * args[2], lobj_t **);
lobj_t * prim_cons(lobj_t * args[2], lobj_t **);
lobj_t * prim_head(lobj_t * args[1], lobj_t **);
lobj_t * prim_tail(lobj_t * args[1], lobj_t **);
lobj_t * prim_eval(lobj_t * args[2], lobj_t **);
lobj_t * prim_apply(lobj_t * args[3], lobj_t **);
lobj_t * prim_globals(lobj_t ** args, lobj_t **);
lobj_t * prim_allocations(lobj_t ** args, lobj_t **);
// lobj_t * prim_print(lobj_t * args[1]);
lobj_t * form_def(lobj_t * args[2], lobj_t **);
lobj_t * form_setq(lobj_t * args[2], lobj_t **);
lobj_t * form_quote(lobj_t * args[1], lobj_t **);
lobj_t * form_if(lobj_t * args[3], lobj_t **);
lobj_t * form_fn(lobj_t * args[2], lobj_t **);
lobj_t * form_do(lobj_t * args[1], lobj_t **);

/* Macros */
#define LASSERT(cond, fmt, ...)                     \
  if (!(cond)) { ROOT = NIL;                        \
      CURRENT_ERROR = new_err(fmt, ##__VA_ARGS__);  \
      longjmp(TOPLEVEL, 1); }

#define LRAISE(fmt, ...) \
  ({ CURRENT_ERROR = new_err(fmt, ##__VA_ARGS__); longjmp(TOPLEVEL, 1); })
 
// Accessors and mutators for data types. Those prefixed with f are unsafe but faster
#define car(pair)            (tocons(pair)->_car)
#define cdr(pair)            (tocons(pair)->_cdr)
#define setcar(pair, value)  (tocons(pair)->_car = (value))
#define setcdr(pair, value)  (tocons(pair)->_cdr = (value))
#define fcar(pair)           (((cons_t*)(pair))->_car)
#define fcdr(pair)           (((cons_t*)(pair))->_cdr)
#define fsetcar(pair, v)     (((cons_t*)(pair))->_car = (v))
#define fsetcdr(pair, v)     (((cons_t*)(pair))->_cdr = (v))

// Macros for comparing symbols and environments. Those prefixed with f are unsafe but faster
#define cmpsym(sym1, sym2)   (strcmp(tosym(sym1)->name, tosym(sym2)->name))
#define cmpstrsym(str, sym)  (strcmp((str), tosym(sym)->name))
#define cmpsymstr(sym, str)  (strcmp(tosym(sym)->name, (str)))
#define fcmpsym(sym1, sym2)  (strcmp(((sym_t*)(sym1))->name, ((sym_t*)(sym2))->name))
#define fcmpstrsym(str, sym) (strcmp((str), ((sym_t*)(sym))->name))
#define fcmpsymstr(sym, str) (strcmp(((sym_t*)(sym))->name, str))

#endif
