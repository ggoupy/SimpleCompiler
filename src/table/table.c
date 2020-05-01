#include "../parser.h"
#include "table.h"

size_t symbols_next = 0;
size_t active_next = 0;
size_t functions_next = 0;
size_t arg_count = 0;
size_t prev_arg_count = 0;
symbol_t *active_table = NULL;
fun_symbol_t *scope = NULL;

int is_declared(const symbol_t table[], const size_t next_index, const char *var_name) {
	for(int i = 0; i < next_index; i++) {
		if(strcmp(var_name, table[i].name) == 0)
			return 1;
	}
	
	return 0;
}

void check_valid_use(const symbol_t table[], const size_t next_index, const char *var_name) {
	if(!is_declared(table, next_index, var_name)) {
		char *err_msg;
		if(asprintf(&err_msg, "use of undeclared variable \"%s\"", var_name) < 0) {
			printf("Failed while writing error msg\n");
			exit(0);
		}
		
		exit(yyerror(err_msg));
	}
}

void check_use_scoped(const char *var_name) {
	int declared = 0;
	
	if(scope != NULL) // we're inside a function, check on local scope
		declared = is_declared(scope->symbols_template, scope->symbols_next, var_name);
	
	if(!declared) // haven't found var, check on global scope
		declared = is_declared(symbols, symbols_next, var_name);
	
	if(!declared) { // didn't found at reachable scope, crash
		char *err_msg;
		if(asprintf(&err_msg, "use of undeclared variable \"%s\" in reachable scope", var_name) < 0) {
			printf("Failed while writing error msg\n");
			exit(0);
		}
		
		exit(yyerror(err_msg));
	}
}

void check_valid_declaration(const symbol_t table[], const size_t next_index, const char *var_name) {
	if(is_declared(table, next_index, var_name)) {
		char *err_msg;
		if(asprintf(&err_msg, "redeclaration of variable \"%s\"", var_name) < 0) {
			printf("Failed while writing error msg\n");
			exit(0);
		}
		
		exit(yyerror(err_msg));
	}
}

void check_declaration_scoped(const char *var_name) {
	// TO DO: provide scope info on declaration errors
	
	if(scope != NULL) // we're inside a function declaration
		check_valid_declaration(scope->symbols_template, scope->symbols_next, var_name);
	else // we're on the global variables declarations
		check_valid_declaration(symbols, symbols_next, var_name);
}

void print_symbol_table(const symbol_t table[], const size_t next_index) {
	printf("SYMBOLS\n");
	printf("+%s+%s+\n", "----------------", "-----------");
	printf("| %-15s|%10s |\n", "Name", "Type");
	printf("+%s+%s+\n", "----------------", "-----------");
	
	size_t count = 0;
	while(count < next_index) {
		printf("| %-15s|%10s |\n", table[count].name, type_to_string(table[count].type));
		printf("+%s+%s+\n", "----------------", "-----------");
		
		count++;
	}
}

void print_function_symbol(const fun_symbol_t * funsymb) {
	// TO DO: implement
	printf("SHOULD PRINT FUNCTION SYMBOL\n");
}

void print_functions_table() {
	// TO DO: implement
	printf("SHOULD PRINT FUNCTIONS TABLE\n");
}

symbol_t new_symbol(const char *name, const int type) {
	if(name[0] != '_')
		assert(type == TYPE_INT || type == TYPE_FLOAT);
	
	char *name_copy = strdup(name);
	symbol_t item;
	
	item.name = name_copy;
	item.type = type;
	value_t val;
	switch(type) {
		case TYPE_INT:
		case TYPE_VOID:
			val.int_val = 0;
			break;
		case TYPE_FLOAT:
			val.float_val = 0;
			break;
		default:
			exit(yyerror(""));
			break;
	}
	
	item.value = val;
	return item;
}

fun_symbol_t new_function_symbol(const char *name) {
	assert(name != NULL);
	char *name_copy = strdup(name);
	
	fun_symbol_t item;
	item.name = name_copy;
	item.type = TYPE_VOID;
	item.symbols_template = (symbol_t *)calloc(TABLE_SIZE, sizeof(symbol_t));
	item.symbols_next = 1; // 0 is reserved for return value
	item.param_count = 0;
	item.tree = NULL;
	
	item.symbols_template[0] = new_symbol("_return", TYPE_VOID);
	
	return item;
}

void add_symbol(symbol_t table[], size_t *next_index_ptr, symbol_t item) {
	if(*next_index_ptr == TABLE_SIZE) {
		if(debug_enabled) {
			printf("Table size: %d\n", TABLE_SIZE);
			print_symbol_table(table, *next_index_ptr);
			printf("Attempted to create variable \"%s\" of type %s\n", item.name, type_to_string(item.type));
		}
		exit(yyerror("symbol table overflow"));
	}
	
	table[*next_index_ptr] = item;
	if(debug_enabled)
		printf("Added symbol \"%s\" [%p]\n", item.name, &table[*next_index_ptr]);
	
	(*next_index_ptr)++;
}

void add_scoped_symbol(symbol_t item) {
	if(scope != NULL) // we're inside a function declaration
		add_symbol(scope->symbols_template, &(scope->symbols_next), item);
	else // we're declaring global variables
		add_symbol(symbols, &symbols_next, item);
}

void add_function(fun_symbol_t item) {
	if(functions_next == TABLE_SIZE) {
		if(debug_enabled) {
			printf("Table size: %d\n", TABLE_SIZE);
			print_functions_table();
			printf("Attempted to create function \"%s\"\n", item.name);
		}
		exit(yyerror("symbol table overflow"));
	}
	
	functions[functions_next] = item;
	if(debug_enabled)
		printf("Added function \"%s\" [%p]\n", item.name, &functions[functions_next]);
	
	functions_next++;
}

symbol_t *lookup_symbol(symbol_t table[], const size_t next_index, const char *identifier) {	
	symbol_t *symbol_ptr = NULL;
	for(int i = 0; i < next_index; i++) {
		symbol_t current_symbol = table[i];
		
		if(strcmp(current_symbol.name, identifier) == 0) {
			symbol_ptr = &table[i];
		}
	}
	
	if(debug_enabled) {
		if(symbol_ptr != NULL)
			printf("found at %p. \n", symbol_ptr);
		else
			printf("not found. \n");
	}
	
	return symbol_ptr;
}

symbol_t *lookup_symbol_scoped(const char *identifier) {
	if(debug_enabled)
		printf("Looking up variable \"%s\"... ", identifier);
	
	symbol_t *symbol_ptr = NULL;
	if(scope != NULL) { // we're inside a function, try local scope first
		if(debug_enabled)
			printf("in local scope: ");
		
		symbol_ptr = lookup_symbol(scope->symbols_template, scope->symbols_next, identifier);
	}
	
	if(symbol_ptr == NULL) { // we haven't found the symbol yet, try on global scope
		if(debug_enabled)
			printf("in global scope: ");
		symbol_ptr = lookup_symbol(symbols, symbols_next, identifier);
	}
	
	return symbol_ptr;
}

symbol_t *lookup_symbol_exec(const char *identifier) {
	if(debug_enabled)
		printf("Exec: looking up variable \"%s\"... ", identifier);
	
	symbol_t *symbol_ptr = NULL;
	if(active_table != NULL) { // we're executing a function, try local scope first
		if(debug_enabled)
			printf("in local scope: ");
		
		symbol_ptr = lookup_symbol(active_table, active_next, identifier);
	}
	
	if(symbol_ptr == NULL) { // we haven't found the symbol yet, try on global scope
		if(debug_enabled)
			printf("in global scope: ");
		symbol_ptr = lookup_symbol(symbols, symbols_next, identifier);
	}
	
	return symbol_ptr;
}

fun_symbol_t *lookup_function(const char *identifier) {
	if(debug_enabled)
		printf("Looking up function \"%s\"... ", identifier);
	
	fun_symbol_t *function_ptr = NULL;
	for(int i = 0; i < functions_next; i++) {
		fun_symbol_t current_symbol = functions[i];
		
		if(strcmp(current_symbol.name, identifier) == 0) {
			function_ptr = &functions[i];
		}
	}
	
	if(debug_enabled) {
		if(function_ptr != NULL)
			printf("Found at %p\n", function_ptr);
		else
			printf("Not found\n");
	}
	
	return function_ptr;
}
