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
#include "mpc.h"

void intern_obj(lobj_t * new, env_t * env, lobj_t * binding) {
  intern(tosym(new), env, binding);
}

void initialize_lisp() {
  CURRENT_ERROR = NULL;
  ROOT = NULL;
  ALLOCATIONS = 1;

  sym_t * nil = mk_sym("nil");
  lobj_t * nil_ = LOBJ_CAST(nil);
  NIL = nil_;
  GLOBALS = mk_env("nil", nil_);
  ALLOC = LOBJ_CAST(GLOBALS);
  LINK(nil_);

  intern_obj(new_sym("+"), GLOBALS, new_proc(prim_add, GLOBALS, 2));
  intern_obj(new_sym("-"), GLOBALS, new_proc(prim_sub, GLOBALS, 2));
  intern_obj(new_sym("*"), GLOBALS, new_proc(prim_mul, GLOBALS, 2));
  intern_obj(new_sym("/"), GLOBALS, new_proc(prim_div, GLOBALS, 2));
  intern_obj(new_sym("cons"), GLOBALS, new_proc(prim_cons, GLOBALS, 2));
  intern_obj(new_sym("head"), GLOBALS, new_proc(prim_head, GLOBALS, 1));
  intern_obj(new_sym("tail"), GLOBALS, new_proc(prim_tail, GLOBALS, 1));
  intern_obj(new_sym("def"), GLOBALS, new_proc(prim_def, GLOBALS, 3));
  intern_obj(new_sym("eval"), GLOBALS, new_proc(prim_eval, GLOBALS, 2));
  intern_obj(new_sym("apply"), GLOBALS, new_proc(prim_apply, GLOBALS, 2));
  intern_obj(new_sym("globals"), GLOBALS, new_proc(prim_globals, GLOBALS, 0));
  intern_obj(new_sym("allocations"), GLOBALS, new_proc(prim_allocations, GLOBALS, 0));
  return;
}

int main(int argc, char** argv) {

  initialize_lisp();
  
  mpc_parser_t * Number  = mpc_new("number");
  mpc_parser_t * Symbol  = mpc_new("symbol");
  mpc_parser_t * Quote   = mpc_new("quote");
  mpc_parser_t * List    = mpc_new("list");
  mpc_parser_t * Sexpr   = mpc_new("sexpr");
  mpc_parser_t * Cons    = mpc_new("cons");
  mpc_parser_t * Expr    = mpc_new("expr");
  mpc_parser_t * Rascal  = mpc_new("rascal");
  
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                                      \
      number  : /-?[0-9]+/ ;                                                               \
      symbol  : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;                                         \
      quote   : ':' <expr>{1} ;                                                            \
      list    : '[' <expr>* ']' ;                                                          \
      sexpr   : '(' <expr>* ')' ;                                                          \
      cons    : <list> | <sexpr> ;                                                         \
      expr    : <number> | <symbol> | <cons> | <quote> ;                                   \
      rascal  : /^/ <expr>* /$/ ;                                                          \
    ",
      Number, Symbol, Quote, List, Sexpr, Cons, Expr, Rascal);
  
  puts("Rascal Version 0.0.0.0.6");
  puts("Press Ctrl+c to Exit\n");
  
  while (1) {
    if (setjmp(TOPLEVEL)) lobj_println(CURRENT_ERROR);

    char* input = readline("rascal> ");
    mpc_result_t r;

    if (mpc_parse("<stdin>", input, Rascal, &r)) {
      //show_tree(r.output, 0);
      ROOT = lobj_eval(lobj_read(r.output, GLOBALS), GLOBALS);
      lobj_println(ROOT);

	
      if (ALLOCATIONS > ALLOCATIONS_LIMIT) {
	gc();
	printf("Allocations after gc: %d\n", ALLOCATIONS);
      }

      mpc_ast_delete(r.output);
    } else {    
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
    
  }
  
  mpc_cleanup(8, Number, Symbol, Quote,  List, Sexpr, Cons, Expr, Rascal);  
  
  return 0;
}
