#ifndef reader_h
#define reader_h
#include "rascal.h"
#include "object.h"
#include "mpc.h"

/* Forward declarations  */
char * readline(char *);
lobj_t * lobj_read_num(mpc_ast_t *);
lobj_t * lobj_read(mpc_ast_t *, env_t *);
lobj_t * lobj_read_quote(mpc_ast_t *, env_t *);

#endif

