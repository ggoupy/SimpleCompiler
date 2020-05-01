#define TABLE_SIZE 20

/* EXTERNS */
int yyerror(const char *);
void yywarning(const char *);
char *type_to_string(const int);
extern int debug_enabled;
// Index for next symbol on global table
extern size_t symbols_next;
extern size_t active_next;
extern size_t functions_next;

/* FUNCTIONS */
void print_functions_table();

/* STRUCTS */
typedef union value {
	int int_val;
	float float_val;
} value_t;

typedef struct symbol_item {
	const char *name;
	int type;
	value_t value;
} symbol_t;

typedef struct param_node {
	const char *name;
	struct param_node *next;
} param_node_t;

typedef struct fun_symbol_item {
	const char *name;
	int type;
	symbol_t *symbols_template;
	size_t symbols_next;
	size_t param_count;
//	param_node_t *params;
	struct node *tree;
} fun_symbol_t;

/* STRUCT FUNCTIONS */
symbol_t new_symbol(const char *, const int);
fun_symbol_t new_function_symbol(const char *);
void print_symbol_table(const symbol_t[], const size_t);
void add_symbol(symbol_t[], size_t *, symbol_t);
void add_scoped_symbol(symbol_t);
void add_function(fun_symbol_t);
symbol_t *lookup_symbol(symbol_t[], const size_t, const char *);
symbol_t *lookup_symbol_scoped(const char *);
symbol_t *lookup_symbol_exec(const char *);
fun_symbol_t *lookup_function(const char *);
int is_declared(const symbol_t[], const size_t, const char *);
void check_valid_declaration(const symbol_t[], const size_t, const char *);
void check_declaration_scoped(const char *);
void check_valid_use(const symbol_t[], const size_t, const char *);
void check_use_scoped(const char *);
void print_function_symbol(const fun_symbol_t *); //TODO
void print_functions_table(); //TODO


/* VARIABLES */
// Global symbols table
symbol_t symbols[TABLE_SIZE];
// Functions table
fun_symbol_t functions[TABLE_SIZE];
// This variable will keep a reference to the active symbols table, so accesses
// and assigns are attempted at the local scope first. When NULL, the operations
// will be performed over the global symbols table directly. Local tables are
// templated on function declaration parsing and instantiated on runtime.
extern symbol_t *active_table;
// This variable will keep a reference to the active scope. When NULL, the scope
// is the global scope.
extern fun_symbol_t *scope;
