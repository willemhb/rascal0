#ifndef vm_h

#define vm_h
#include "rascal.h"
#include "object.h"

#define STACKSIZE 2048 * 2048
// Argument registers
int64_t ACCUM;
// Register for the current environment
lobj_t * ENV;

lobj_t * STACK[STACKSIZE];

#endif
