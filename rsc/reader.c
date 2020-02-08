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

lobj_t * lobj_read(mpc_ast_t * t, sym_t * env) {
  // Ignore root
  if (streq(t->tag, ">")) return lobj_read(t->children[1], env);

  // Evaluate atoms directly
  if (strstr(t->tag, "quote")) return lobj_read_quote(t->children[1], env);
  if (strstr(t->tag, "number")) return lobj_read_num(t);
  if (strstr(t->tag, "symbol")) return new_sym(t->contents, NULL);

  
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
      output->quote = 1;
    }

    output->quote = strstr(t->tag, "list") ? 1 : 0;
  }
  return output;
}

lobj_t * lobj_read_quote(mpc_ast_t * t, sym_t * env) {
  // Quoted numbers are read directly; quoted numbers are not distinguished from regular numbers
  // for now.
  if (strstr(t->tag, "number")) return lobj_read_num(t);

  lobj_t * output;
  if (strstr(t->tag, "symbol")) output = new_sym(t->contents, NULL);
  /* If sexpr or list, then create an empty list  */
  if (strstr(t->tag, "cons")) {
    output = NIL;
    mpc_ast_t * child;

    // For sexpr, the children should *not* be quoted; for quoted lists, they should be.
    if (strstr(t->tag, "sexpr")) {
      for (int i = t->children_num-1; i > 0; i--) {
	child = t->children[i];
	if (streq(child->contents, "(")) continue;
	if (streq(child->contents, ")")) continue;
	if (streq(child->contents, "[")) continue;
	if (streq(child->contents, "]")) continue;
	if (streq(child->tag, "regex")) continue;

      output = new_cons(lobj_read(child, env), output);
      output->quote = 1;
    }
    } else {
      for (int i = t->children_num-1; i > 0; i--) {
	child = t->children[i];
	if (streq(child->contents, "(")) continue;
	if (streq(child->contents, ")")) continue;
	if (streq(child->contents, "[")) continue;
	if (streq(child->contents, "]")) continue;
	if (streq(child->tag, "regex")) continue;

      output = new_cons(lobj_read_quote(child, env), output);
      // Lists should be quoted recursively.
      output->quote = 1;
      }
    }
  }

  output->quote = 1;
  return output;
}
