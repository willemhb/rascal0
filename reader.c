#include "reader.h"

/* Reader  */
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(BUFFER, 2048, stdin);
  char* cpy = malloc(strlen(BUFFER)+1);
  strcpy(cpy, BUFFER);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}


lobj_t * lobj_read_num(mpc_ast_t * t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? new_num(x) : new_err("Invalid Number");
}

lobj_t * lobj_read(mpc_ast_t * t, env_t * env) {
  // Ignore root
  if (streq(t->tag, ">")) return lobj_read(t->children[1], env);

  // Evaluate atoms directly
  if (strstr(t->tag, "number")) return lobj_read_num(t);
  if (strstr(t->tag, "symbol")) return new_sym(t->contents);

  
  /* If sexpr or list, then create an empty list  */
  lobj_t * output = NIL;
  if (strstr(t->tag, "cons")) {
    mpc_ast_t * child;

    for (int i = t->children_num-1; i > 0; i--) {
      child = t->children[i];
      if (streq(child->contents, "(")) continue;
      if (streq(child->contents, ")")) continue;
      if (streq(child->contents, "[")) continue;
      if (streq(child->contents, "]")) continue;
      if (streq(child->tag, "regex")) continue;

      output = new_cons(lobj_read(child, env), output);
    }

    if (strstr(t->tag, "sexpr")) {
      output->type = LOBJ_SEXPR;
    }
  }

  return output;
}
