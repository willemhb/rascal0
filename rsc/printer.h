#ifndef printer_h

#define printer_h
#include "rascal.h"
#include "object.h"

/* Forward declarations  */
// Printer
void lobj_print(lobj_t *);
void lobj_expr_print(lobj_t *, char, char);
void lobj_println(lobj_t *);

// Debug
void show_proc_info(prim_t *);
void show_alloc_list();
void show_globals();

#endif
