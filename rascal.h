
#ifndef rascal_h
#define rascal_h

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include "mpc.h"

// Forward type declarations
typedef struct _lobj_t lobj_t;
typedef struct _err_t err_t;
typedef struct _num_t num_t;
typedef struct _sym_t sym_t;
typedef struct _cons_t cons_t;
typedef struct _env_t env_t;
typedef lobj_t * (*proc_t) (lobj_t**);
typedef struct _lambda_t lambda_t;

/* Global variables  */
// Memory and stack management, environment
env_t * GLOBALS;
// Head of the list of all allocated objects
lobj_t * ALLOC;
// Head of all reachable objects
lobj_t * ROOT;
lobj_t * CURRENT_ERROR;
jmp_buf TOPLEVEL;
/* 
Nil and nil checks 
For now Nil is a unique symbol created when the interpreter is initialized.
Its address is recorded and used to check for nil, and it is stored in a global variable
for easy access. This system is clumsy but easier and less error prone for now than the
current implementation (all references to nil lookup nil in the global environment).
This is a target for  optimization once the interpreter is fleshed out and working pretty well.
*/
// Reference to unique NIL object
lobj_t * NIL;
int ALLOCATIONS;
// Arbitrary allocation limit (should research a good one)
#define ALLOCATIONS_LIMIT 128
// Read buffer
char BUFFER[2048];
// Add an object to the linked list of allocated objects.
#define LINK(ob)         \
  do {                   \
      ob->next = ALLOC ; \
      ALLOC = ob ;       \
      ALLOCATIONS++ ;    \
       } while (0)


// String utilities
#define streq(s1, s2) (strcmp((s1),(s2))==0)
#define str_init(s1, s2) ({ (s1) = malloc(strlen((s2)) + 1); strcpy((s1), (s2)); })
#define strterm(s) ((s)[strlen((s))])

#endif
