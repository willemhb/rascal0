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

void intern_obj(lobj_t * new, sym_t * env, lobj_t * binding) {
  intern(tosym(new), env, binding);
}

void initialize_lisp() {
  CURRENT_ERROR = NULL;
  ROOT = NULL;
  UNBOUND = NULL;
  ALLOCATIONS = 1;

  GLOBALS = mk_sym("nil", NULL);
  NIL = LOBJ_CAST(GLOBALS);
  // Nil must be self-evaluating.
  tosym(NIL)->binding = NIL;
  NIL->quote = 1;
  ALLOC = NIL;

  intern_obj(new_sym("+", NULL), GLOBALS, new_prim(prim_add,  2));
  intern_obj(new_sym("-", NULL), GLOBALS, new_prim(prim_sub, 2));
  intern_obj(new_sym("*", NULL), GLOBALS, new_prim(prim_mul, 2));
  intern_obj(new_sym("/", NULL), GLOBALS, new_prim(prim_div, 2));
  intern_obj(new_sym("cons", NULL), GLOBALS, new_prim(prim_cons, 2));
  intern_obj(new_sym("head", NULL), GLOBALS, new_prim(prim_head, 1));
  intern_obj(new_sym("tail", NULL), GLOBALS, new_prim(prim_tail, 1));
  intern_obj(new_sym("def", NULL), GLOBALS, new_prim(prim_def, 3));
  intern_obj(new_sym("eval", NULL), GLOBALS, new_prim(prim_eval, 2));
  intern_obj(new_sym("apply", NULL), GLOBALS, new_prim(prim_apply, 3));
  intern_obj(new_sym("globals", NULL), GLOBALS, new_prim(prim_globals, 0));
  intern_obj(new_sym("allocations", NULL), GLOBALS, new_prim(prim_allocations, 0));
  intern_obj(new_sym("setq", NULL), GLOBALS, new_prim(prim_setq, 3));
  intern_obj(new_sym("fn", NULL), GLOBALS, new_prim(prim_fn, 3));
  return;
}

int main(int argc, char** argv) {

  initialize_lisp();
  
  mpc_parser_t * Number   = mpc_new("number");
  mpc_parser_t * Symbol   = mpc_new("symbol");
  mpc_parser_t * Quote    = mpc_new("quote");
  mpc_parser_t * List     = mpc_new("list");
  mpc_parser_t * Sexpr    = mpc_new("sexpr");
  mpc_parser_t * Cons     = mpc_new("cons");
  mpc_parser_t * Expr     = mpc_new("expr");
  mpc_parser_t * Rascal   = mpc_new("rascal");
  
  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                                       \
      number   : /-?[0-9]+/ ;                                                               \
      symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;                                         \
      quote    : ':' <expr> ;                                                               \
      list     : '[' <expr>* ']' ;						            \
      sexpr    : '(' <expr>* ')' ;                                                          \
      cons     : <list> | <sexpr> ;                                                         \
      expr     : <number> | <symbol> | <cons> | <quote> ;                                   \
      rascal   : /^/ <expr>* /$/ ;                                                          \
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
      ROOT = lobj_read(r.output, GLOBALS);
      lobj_println(ROOT);
      lobj_println(lobj_eval(ROOT, GLOBALS));

	
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
  
  mpc_cleanup(8, Number, Symbol,  Quote,  List, Sexpr, Cons, Expr, Rascal);  
  
  return 0;
}
