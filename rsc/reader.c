#include "reader.h"

/* Reader  */
char nextchar(FILE * f) {
  char c;
  int ch;

  do {
      ch = fgetc(f);
      if (ch == EOF) return 0;

      c = (char)ch;
      if (c == ';') {
        do {
             ch = fgetc(f);
             if (ch == EOF) return 0;
           } while ((char)ch != '\n');

      c = (char)ch;
      }
    } while (isspacec(c));
    return c;
}

void accumchar(char c, int *pi){
  READ_BUFFER[(*pi)++] = c;
  if (*pi >= (int)(sizeof(READ_BUFFER)-1)) {
    puts("Warning: read buffer overflow.");
  }
}

int read_token(FILE * f, char c) {
  int i = 0, ch;

    ungetc(c, f);

    while (1) {
      ch = fgetc(f);

      if (ch == EOF) break;

      c = (char)ch;
      
      if (!issymc(c)) break;

      accumchar(c, &i);
    }

 ungetc(c, f);
 
 READ_BUFFER[i++] = '\0';
 return i;
}

uint32_t peek(FILE * f) {
    char c, *end;
    long x;

    if (TOKTYPE != TOK_NONE) return TOKTYPE;

    c = nextchar(f);

    if (feof(f)) return TOK_NONE;

    if (strchr("([", c) != NULL) TOKTYPE = TOK_OPEN;

    if (strchr(")]", c) != NULL) TOKTYPE = TOK_CLOSE;

    if (c == ':') TOKTYPE = TOK_QUOTE;

    if (c == '$') TOKTYPE = TOK_UNQUOTE; 

    if (c == '"') TOKTYPE = TOK_STR;
    
    if (issymc(c)) {
        read_token(f, c);
	

	// Simplest way to handle "-" being technically a numerical string.
	if (streq(READ_BUFFER, "-")) {
	  TOKTYPE = TOK_SYM;
	  TOKVAL = new_sym(READ_BUFFER);
	} else  if (isnums(READ_BUFFER)) {
	  TOKTYPE =  TOK_NUM;
	    x = strtol(READ_BUFFER, &end, 10);
	    if (*end != '\0') {
	      TOKTYPE = TOK_ERROR;
	      TOKVAL = new_err("Bad number input.");
	    }
	    
	    TOKVAL = new_num(x);
	    
        } else {
            TOKTYPE = TOK_SYM;
            TOKVAL = new_sym(READ_BUFFER);
        }
    }
    return TOKTYPE;
}

// build a list of conses.
lobj_t * read_list(FILE *f) {
  lobj_t * out = NIL, ** curr = &out;
  uint32_t t = peek(f);
  
  while (t != TOK_CLOSE) {
    LASSERT(!feof(f), "read error: unexpected end of input.")
    *curr = new_cons(NIL, NIL);
    setcar(*curr, read_expr(f));
    curr = &cdr(*curr);    
    t = peek(f);
  }
    take();
    return out;
}

lobj_t * read_str(FILE *f) {
  int i = 0, c;
  char ch = fgetc(f);

    while (ch != '"') {
      LASSERT(ch != EOF, "Unexpected EOF in string literal.")
      c = (char)ch;
      accumchar(c, &i);
      ch = fgetc(f);
    }
 
 READ_BUFFER[i++] = '\0';
 return new_str(READ_BUFFER); 
}

lobj_t * read_expr(FILE *f) {
  if (feof(f)) return NIL;
    switch (peek(f)) {
    case TOK_CLOSE:
        take();
        LRAISE("read error: unexpected ')'\n");
    case TOK_QUOTE:{
      take();
      return new_cons(new_sym("quote"), new_cons(read_expr(f), NIL));
    }
    case TOK_UNQUOTE:{
      take();
      return new_cons(new_sym("unquote"), new_cons(read_expr(f), NIL));
    } 
    case TOK_SYM:
    case TOK_NUM:
        take();
        return TOKVAL;
    case TOK_OPEN:{
        take();
        return read_list(f);
    }case TOK_STR:{
       take();
       return read_str(f);
     }
    }

    return NIL;
}


lobj_t * load_lisp_file(char * fname, lobj_t ** env) {
  lobj_t * e, * v = NIL;
  LASSERT(strstr(fname, ".rsp"), "Invalid filename")
  FILE * f = fopen(fname, "r");
  LASSERT(f != NULL, "File not found.")  
    while (1) {
      e = read_expr(f);
      if (feof(f)) break;
      v = lobj_eval(e, env);
    }
    fclose(f);
    return v; 
}
