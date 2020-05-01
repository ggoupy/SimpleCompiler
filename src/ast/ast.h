// Externs
extern char* token_to_string(const int);
extern int exit_error(const char*);
extern int already_returned;

// AST nodes
typedef struct node {
	int node_class;
	int type;
	
	struct node *a; // Will be casted to char * for var, value_t * for const, fun_symbol_t * for call
	struct node *b;
	struct node *c;
} node_t;

static node_t *tree = NULL;

void append_node(node_t *, node_t *);
void print_tree(node_t *);
node_t *create_node(const int, node_t *, node_t *, node_t *, int);
void type_check(node_t *, node_t *);
value_t execute(node_t *);
