#ifndef reader_h
#define reader_h
#include "rascal.h"
#include "object.h"
#include "eval.h"

#define SYMBOL_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_+-*\\/=<>!&?"
#define NUMBER_CHARS "-0123456789"
#define CONS_CHARS   "()[]"
#define SPACE_CHARS  " \t\v\r\n"

enum { TOK_NONE, TOK_OPEN, TOK_CLOSE, TOK_SYM, TOK_NUM, TOK_ERROR, TOK_QUOTE };
static uint32_t TOKTYPE = TOK_NONE;
static lobj_t * TOKVAL;
static char READ_BUFFER[2048];

static void take() { TOKTYPE = TOK_NONE;  }

/* Forward declarations  */
char nextchar(FILE *);
void accumchar(char, int *);
int read_token(FILE *, char);
uint32_t peek(FILE *);
lobj_t * read_list(FILE *);
lobj_t * read_expr(FILE *);

// Helper macros for testing string characters
#define isnumc(c)   (strchr(NUMBER_CHARS, c))
#define issymc(c)   (strchr(SYMBOL_CHARS, c))
#define isspacec(c) (strchr(SPACE_CHARS, c))
#define issexprc(c) (strchr(CONS_CHARS, c))
#define isnums(s)   (strspn(s, NUMBER_CHARS) == strlen(s))
#define issyms(s)   (strspn(s, SYMBOL_CHARS) == strlen(s))

#endif
