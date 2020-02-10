#include "printer.h"

void lobj_print(lobj_t * v) {
  if (v->quote) { lobj_print_quote(v); return; }
  switch(v->type) {
  case LOBJ_NUM:   printf("%li", tonum(v)->value); break;
  case LOBJ_ERR:   printf("Error: %s", toerr(v)->msg); break;
  case LOBJ_SYM:   printf("%s", tosym(v)->name); break;
  case LOBJ_CONS:  lobj_expr_print(v, '(', ')'); break;
  case LOBJ_PRIM:
  case LOBJ_PROC:  printf("#proc"); break;
  default: printf("#");
  }
}

void lobj_print_quote(lobj_t * v) {
  switch (v->type) {
  case LOBJ_NUM:   printf("%li", tonum(v)->value); break;
  case LOBJ_ERR:   printf("Error: %s", toerr(v)->msg); break;
  case LOBJ_SYM:   printf(":%s", tosym(v)->name); break;
  case LOBJ_CONS:  lobj_expr_print(v, '[', ']'); break;
  case LOBJ_PRIM:
  case LOBJ_PROC:  printf("#proc"); break;
  default: printf("#");
  }
}

void lobj_println(lobj_t * v) { lobj_print(v); putchar('\n'); }

void lobj_expr_print(lobj_t * v, char open, char close) {
  putchar(open);
  lobj_t * curr = v;
  
  while (!isnil(curr)) {
    if (!iscons(curr)) {
      lobj_print(curr);
      break;
    }
    lobj_print(car(curr));
    if (!isnil(cdr(curr))) putchar(' ');
    curr = cdr(curr);
    
  }

  putchar(close);
}


/* Debugging. */
void show_node(mpc_ast_t * t, int tab) {
  char offset[tab+1];
  for (int i = 0; i < tab+1; i++) {
    offset[i] = '\t';
  }
  offset[tab+1] = '\0';
  printf("%s%s | %s\n", offset, t->tag, t->contents);
}

void show_tree(mpc_ast_t * t, int tab) {
  show_node(t, tab);
  for (int i = 0; i < t->children_num; i++) {
    show_tree(t->children[i], tab + 1);
  }
}

void show_proc_info(prim_t * fun) {
  printf("type: %d\n", fun->type);
  printf("arity: %d\n", fun->arity);
  printf("address of body: %p\n", fun->body);
}

void show_alloc_list() {
  lobj_t * curr = ALLOC;
  while (curr) {
    lobj_println(curr);
    curr = curr->next;
  }
}
