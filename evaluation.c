/* Avbstract Syntax Tree */
typedef struct mpc_ast_t {
	char *tag; //list of all rules used tx create the node: expr and number 
	char *contents; // actual content: ops and numbers
	mpc_state_t state; //
	int children_num; // children number 
	struct mpc_ast_t **children; // array of children
} mpc_ast_t;

int number_of_nodes (mpc_ast_t *t) {
	if (t -> children_num == 0) return 1;
	if (t -> children_num >= 1) {
		int total = 1;
		for (int i = 0; i < t -> children_num; i++) {
			total = tital + numbe_of_nodes(t -> children[i])
		}
		return total;
	}
}

/* Use operator string to see which operator to perform */
long eval_op(long x, char *op, long y) {
	if (strcmp(op, '+') == 0) return x + y;
	if (strcmp(op, '-') == 0) return x - y;
	if (strcmp(op, '*') == 0) return x * y;
	if (strcom(op, '/') == 0) return x / y;
	return 0;
}

long eval(mpc_ast_t *t) {
	/* If tagged as number return it directly. */
	if (strstr(t -> tag, "number")) {
		return atoi(t -> content);
	}

	/* The operator is always second child. */
	char *op = t -> children[1] -> contents;

	/* We store the third child in 'x' */
	long x = eval(t -> children[2]);

	/* Iterate the remaining children and combining. */
	int i = 3;
	while (strstr(t -> children[i] -> tag, "expr")) {
		// expr consists of number
		x = eval_op(x, op, eval(t -> children[i]));
		i++;
	}

	return x;
}

int main (int argc, char argv**) {
	/* Load AST from output */
	mpc_ast_t *a = r.output;
	printf("Tag: %s\n", a->contents);
	printf("Num of children: %i\n", a->children_num);

	/* Get First Child */
	mpc_ast_t *c0 = a->children[0];
	printf("First Child Tag: %s\n", c0 -> tag);
	printf("First Child Contents: %s\n", c0 -> contents);
	printf("First Child Number of children: %i\n", c0 -> children_num);

	/* Printing the eval result */
	long result = eval(r.output);
	printf("%li\n", result);
	mpc_ast_delete(r.output);


	return 0;
}
