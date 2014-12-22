#include"mpc.h"
//#include "string.h"
        
#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt){
        fputs(prompt, stdout);
        fgets(buffer, 2048, stdin);
        char *cpy = malloc(strlen(buffer)+1);
        strcpy(cpy, buffer);
        cpy[strlen(cpy) - 1]='\0';
        return cpy;
        }

        void add_history(char*unused){}

        #else
        #include<editline/readline.h>
        #endif

/* Avbstract Syntax Tree 
typedef struct mpc_ast_t {
	char *tag; //list of all rules used tx create the node: expr and number 
	char *contents; // actual content: ops and numbers
	mpc_state_t state; //
	int children_num; // children number 
	struct mpc_ast_t **children; // array of pointers 
} mpc_ast_t; */

int number_of_nodes(mpc_ast_t *t) {
	if (t -> children_num == 0) return 1;
	if (t -> children_num >= 1) {
		int total = 1;
		for (int i = 0; i < t -> children_num; i++) {
			total += number_of_nodes(t -> children[i]);
		}
		return total;
	}
}

/* Declare New lval Struct */
typedef struct lval{
	int type;
	long num;
	/* Error and Symbol types have some string data */
	char *err;
	char *sym;
	/* Cound the Pointer to a list of "lval*" */
	int count;
	struct lval **cell;
} lval;

/* Create Enumeration of Possiible lval Types */
enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

/* Create Enumberation of Possible Error Types */
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

/* Create a pointer to a Number 1val */
lval *lval_num(long x) {
	lval *v = malloc(sizeof(lval));
	v -> type = LVAL_NUM;
	v -> num = x;
	return v;
}

/* Create a pointer to a new Error lval */
lval *lval_err(char *m) {
	lval *v = malloc(sizeof(lval));
	v -> type = LVAL_ERR;
	v -> err = malloc(strlen(m) + 1);
	strcpy(v -> err, m);
		return v;
}

/* Construct a pointer to a num Symbol lval */
lval *lval_sym(char *s) {
	lval *v = malloc(sizeof(lval));
	v -> type = LVAL_SYM;
	v -> sym = malloc(strlen(s) + 1);
	strcpy(v -> sym, s);
	return v;
}

/* A pointer to a new empty Sexpr laval */
lval *lval_sexpr(void) {
	lval *v = malloc(sizeof(lval));
	v -> type = LVAL_SEXPR;
	v -> count = 0;
	v -> cell = NULL;
	return v;
}

void lval_del(lval *v) {
	switch (v -> type) {
		/* Do nothing special for number typr */
		case LVAL_NUM: break;
		case LVAL_ERR: free(v -> err); break;
		case LVAL_SYM: free(v -> sym); break;

					   /* If Sexpr then del all elements inside */
		case LVAL_SEXPR: 
					   for (int i = 0; i < v -> count; i++) {
						   lval_del(v -> cell[i]);
					   }

					   /* Also free the mem allocated to contain the pointers */
					   free(v -> cell);
					   break;
	}

	/* Free the mem allocated for the "lval" struct itself */
	free(v);
}

lval *lval_read_num(mpc_ast_t *t) {
	errno = 0;
	long x = strtol(t -> contents, NULL, 10);
	return errno != ERANGE? lval_num(x): lval_err("Invalid number");
}

/* READING EXPRESSIONS 
 * recursively look at each node of the tree, and construct related lval*
 * using list (*lval) to represent a s-expr */

/* adding a element to the list for s-expr */
lval *lval_add(lval *v, lval *x) {
	v -> count++;
	v -> cell = realloc(v -> cell, sizeof(lval *)  *v -> count);
	v -> cell[v -> count- 1] = x;
	return v;
}

lval *lval_read(mpc_ast_t *t) {
	/* if Symbol or Number return oconversion to that type */
	if (strstr(t -> tag, "number")) return lval_read_num(t);
	if (strstr(t -> tag, "symbol")) return lval_sym(t -> contents);
	
	/* If root (>) or sexpr then create empty list */
	lval *x = NULL;
	if (strcmp(t -> tag, ">") == 0) x = lval_sexpr();
	if (strstr(t -> tag, "sexpr")) x = lval_sexpr();

	/* fill this list with any valid expression contained within */
	for (int i = 0; i < t -> children_num; i++) {
		if (strcmp(t -> children[i] -> contents, "(") == 0) continue;
		if (strcmp(t -> children[i] -> contents, ")") == 0) continue;
		if (strcmp(t -> children[i] -> contents, "}") == 0) continue;
		if (strcmp(t -> children[i] -> contents, "{") == 0) continue;
		if (strcmp(t -> children[i] -> tag, "regex") == 0) continue;
		x = lval_add(x, lval_read(t -> children[i]));
	}

	return x;
}

/* PRINTING EXPRESSIONS */
void lval_print(lval *v);
void lval_expr_print(lval *v, char open, char close) {
	putchar(open);
	for (int i = 0; i < v -> count; i++) {
		/* Print value contained within */
		lval_print(v -> cell[i]);

		if (i != (v -> count - 1)) {
			putchar(' ');
		}
	}
	putchar(close);
}

/* Print an "lval" */
void lval_print(lval *v) {
	switch (v -> type) {
		case LVAL_NUM: printf("%li", v->num); break;
		case LVAL_ERR: /*
					   if(v -> err == LERR_DIV_ZERO) {
						   printf("Error: Division by 0");
					   }
						if (v -> err == LERR_BAD_OP) {
							printf("ERROR: invalid operator!");
						}
							if (v -> err == LERR_BAD_NUM) {
								printf("ERROR: INVALID NUMBER!");
							}*/
					   printf("ERROR!");
							break;
		case LVAL_SYM: printf("%s", v -> sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
	}
}

void lval_println(lval *v) {
	lval_print(v);
	putchar('\n');
}

/* EVALUATION */
/* Use operator string to see which operator to perform 
lval eval_op(lval x, char *op, lval y) {
	/* If either value is an error return it 
	if (x.type == LVAL_ERR) return x;
	if (y.type == LVAL_ERR) return y;

	if (strcmp(op, "+") == 0) return lval_num(x.num + y.num);
	if (strcmp(op, "-") == 0) return lval_num(x.num - y.num);
	if (strcmp(op, "*") == 0) return lval_num(x.num * y.num);
	if (strcmp(op, "/") == 0) {
	/* If sec operand is 0 return error 
	return y.num == 0 ? lval_err(LERR_DIV_ZERO):lval_num(x.num / y.num);
	}
	return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t) {
	/* If tagged as number return it directly. 
	if (strstr(t -> tag, "number")) {
		/* Check if there is some error in conversion 
		errno = 0;
		long x = strtol(t -> contents, NULL, 10);
		return errno != ERANGE ? lval_num(x): lval_err(LERR_BAD_NUM);
	}

	/* The operator is always second child. 
	char *op = t -> children[1] -> contents;

	/* We store the third child in 'x' 
	lval x = eval(t -> children[2]);

	/* Iterate the remaining children and combining. 
	int i = 3;
	while (strstr(t -> children[i] -> tag, "expr")) {
		// expr consists of number
		x = eval_op(x, op, eval(t -> children[i]));
		i++;
	}

	return x;
}
*/

        int main(int argc,char**argv)
        {	    
        /* Create some parsers */
        mpc_parser_t *Number = mpc_new("number");
        mpc_parser_t *Symbol = mpc_new("symbol");
		mpc_parser_t *Sexpr = mpc_new("sexpr");
        mpc_parser_t *Expr = mpc_new("expr");
        mpc_parser_t *Lispy = mpc_new("lispy");
					    
		 /* Define them with the following Language */
mpca_lang(MPCA_LANG_DEFAULT,
		"\
        number: /-?[0-9]+/ ;                     \
        symbol: '+' | '-' | '*' | '/' | '%';   \
		sexpr: '(' <expr>* ')';					\
        expr: <number> | <symbol> | <sexpr>;\
        lispy: /^/ <expr>* /$/; \
        ",
        Number, Symbol, Sexpr, Expr, Lispy);
						    
		
        while(1){
        char*input=readline("lispy> ");
        add_history(input);

		/* Attempt to parse the uesr Input */
        mpc_result_t r;

	if (strcmp(input, "exit") == 0)
			return 0;



        if(mpc_parse("<stdin>", input, Lispy, &r)) {
											/* Load AST from output */
	/*mpc_ast_t *a = r.output;
	printf("Tag: %s\n", a->contents);
	printf("Num of children: %i\n", a->children_num);*/

	/* Get First Child 
	mpc_ast_t *c0 = a->children[0];
	printf("First Child Tag: %s\n", c0 -> tag);
	printf("First Child Contents: %s\n", c0 -> contents);
	printf("First Child Number of children: %i\n", c0 -> children_num);*/

	//printf("num of nodes %i\n", number_of_nodes(&r));

				/* Printing the eval result */
	//lval result = eval(r.output);
	//lval_println(result);
	//mpc_ast_delete(r.output);
	lval *x = lval_read(r.output);
	lval_println(x);
	lval_del(x);

		 /* On Success Pring the AST */
       // mpc_ast_print(r.output);
       // mpc_ast_delete(r.output);
        }else{
								/* Otherwise Pring the Error */
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
        }
        free(input);
        }
								    
								    /* Undefine and delete out parsers */
        mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

        return 0;
        }



