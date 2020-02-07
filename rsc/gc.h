#ifndef gc_h
#define gc_h
#include "rascal.h"
#include "object.h"

/* Forward Declarations  */

// GC & memory management
void lobj_del(lobj_t*);
void gc();
void mark(lobj_t *);
void sweep();

#endif
