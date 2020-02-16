/*

Rascal, a new LISP by Willm.

Initial implementation based on Build Your Own Lisp, but this will diverge (hopefully).

Public Domain.

*/
#include "rascal.h"
#include "object.h"
#include "reader.h"
#include "printer.h"
#include "eval.h"
#include "gc.h"


void initialize_lisp() {
  CURRENT_ERROR = NULL;
  ROOT = NULL;
  ALLOCATIONS = 3;
  

  NIL = LOBJ_CAST(mk_sym("nil"));
  UNBOUND = LOBJ_CAST(mk_sym("undef"));
  TRUE = LOBJ_CAST(mk_sym("t"));
  
  ALLOC = NIL;
  LINK(UNBOUND);
  LINK(TRUE);
  GLOBALS = NIL;

  puts_env(NIL, &GLOBALS, NIL);
  puts_env(UNBOUND, &GLOBALS, UNBOUND);
  puts_env(TRUE, &GLOBALS, TRUE);
  puts_env(new_sym("len"), &GLOBALS, new_prim(prim_len, 1));
  puts_env(new_sym("eq?"), &GLOBALS, new_prim(prim_eq, 2));
  puts_env(new_sym("+"), &GLOBALS, new_prim(prim_add,  2));
  puts_env(new_sym("-"), &GLOBALS, new_prim(prim_sub, 2));
  puts_env(new_sym("*"), &GLOBALS, new_prim(prim_mul, 2));
  puts_env(new_sym("/"), &GLOBALS, new_prim(prim_div, 2));
  puts_env(new_sym("cons"), &GLOBALS, new_prim(prim_cons, 2));
  puts_env(new_sym("head"), &GLOBALS, new_prim(prim_head, 1));
  puts_env(new_sym("tail"), &GLOBALS, new_prim(prim_tail, 1));
  puts_env(new_sym("eval"), &GLOBALS, new_prim(prim_eval, 2));
  puts_env(new_sym("apply"), &GLOBALS, new_prim(prim_apply, 3));
  puts_env(new_sym("globals"), &GLOBALS, new_prim(prim_globals, 0));
  puts_env(new_sym("allocations"), &GLOBALS, new_prim(prim_allocations, 0));
  //  puts_env(new_sym("print"), &GLOBALS, new_prim(prim_print, 1));
  puts_env(new_sym("def"), &GLOBALS, new_prim(form_def, 2));
  puts_env(new_sym("setq"), &GLOBALS, new_prim(form_setq, 2));
  puts_env(new_sym("quote"), &GLOBALS, new_prim(form_quote, 1));
  puts_env(new_sym("if"), &GLOBALS, new_prim(form_if, 3));
  puts_env(new_sym("fn"), &GLOBALS, new_prim(form_fn, 2));
  puts_env(new_sym("do"), &GLOBALS, new_prim(form_do, 1));

  lobj_println(GLOBALS);
  return;
}

int main(int argc, char** argv) {

  initialize_lisp();
  puts("Rascal Version 0.0.0.0.8");
  puts("Press ctrl+c to Exit\n");

  if (argc > 1) {
    return 0;
  }
  
  while (1) {
    if (setjmp(TOPLEVEL)) lobj_println(CURRENT_ERROR);
    printf("rascal> ");
    ROOT = read_expr(stdin);
    if (feof(stdin)) break;
    lobj_println(ROOT);
    lobj_println(lobj_eval(ROOT, &GLOBALS));

    if (ALLOCATIONS > ALLOCATIONS_LIMIT) gc();
  }
  
  return 0;
}
