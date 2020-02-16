#include "gc.h"


void lobj_del(lobj_t * obj) {
  // Ignore null pointers
  if (obj == NULL) return;

  switch (obj->type) {
  case LOBJ_SYM:{
    sym_t * body = tosym(obj);
    free(body->name);
    free(body);
    break;
   }case LOBJ_PROC:{
      lambda_t * pbody = toproc(obj);
      free(pbody);
      break;
    }
  case LOBJ_NUM: free(tonum(obj)); break;
  case LOBJ_ERR:{
    err_t * body = toerr(obj);
    free(body->msg);
    free(body);
    break;
   }case LOBJ_CONS:{
    cons_t * body = tocons(obj);
    free(body);
    break;
    }case LOBJ_FORM:{
     form_t * f = toform(obj);
     free(f->body);
     free(f);
     break;
     }
  }

  ALLOCATIONS--;
}


void mark(lobj_t * obj) {
   // Ignore null pointers and red objects.
  if (obj == NULL) return;
  // This object has already been tagged
  if (obj->tag == GC_BLACK) return;

  obj->tag = GC_BLACK;

  switch (obj->type) {
    // Case 1: atomic objects (no references)
  case LOBJ_NUM:
  case LOBJ_ERR:
  case LOBJ_SYM:
  case LOBJ_PRIM:
  case LOBJ_FORM:
    break;
    // Case 2: pairs
  case LOBJ_CONS:{
    mark(fcar(obj));
    mark(fcdr(obj));
    break;
     // Case 4: procedures
   }case LOBJ_PROC:{
      lambda_t * lmb = toproc(obj);
      mark(LOBJ_CAST(lmb->env));
      mark(LOBJ_CAST(lmb->body));
      break;
     }
  }
}

void sweep() {
  lobj_t * deathrow, * prev = ALLOC, * curr = ALLOC;
  while (curr) {
    if (curr->tag == GC_WHITE)  {
      deathrow = curr;
      prev->next = curr->next;
      curr = curr->next;
      lobj_del(deathrow);
      continue;
    }
    curr->tag = GC_WHITE;
    prev = curr;
    curr = curr->next;
    }
  }

  void gc() {
    mark(LOBJ_CAST(GLOBALS));
    mark(ROOT);
    sweep();
  }
  
