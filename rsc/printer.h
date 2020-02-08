#ifndef printer_h

#define printer_h
#include "rascal.h"
#include "object.h"
#include "mpc.h"

/* Forward declarations  */
// Printer
void lobj_print(lobj_t *);
void lobj_print_quote(lobj_t *);
void lobj_expr_print(lobj_t *, char, char);
void lobj_println(lobj_t *);

// Debug
void show_node(mpc_ast_t *, int);
void show_tree(mpc_ast_t *, int);
void show_proc_info(lambda_t *);
void show_alloc_list();

#endif
