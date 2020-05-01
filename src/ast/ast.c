int already_returned = 0;

void print_node_header(const node_t *node) {
	printf("NODE - %s", token_to_string(node->node_class));
	if(debug_enabled)
		printf(" [%p]", node);
	printf("\n");
	printf("Type: %s\n", type_to_string(node->type));
}

void print_node(const node_t *node) {
	if(node == NULL) {
		if(debug_enabled)
			printf("NULL NODE\n");
		
		return;
	}
	
	print_node_header(node);
	switch(node->node_class) {
		case SEMICOLON: {
			printf("Statement: %p [node %s]\n", node->a, token_to_string(node->a->node_class));
			printf("Next: %p\n", node->b);
			
			break;
		}
		case ARG: {
			printf("Expression: %p [node %s]\n", node->a, token_to_string(node->a->node_class));
			printf("Next: %p\n", node->b);

			break;
		}
		// UNARY NODES
		case PRINT:
		case READ:
		case TILDE:
		case RETURN: {
			printf("Operand: %p [node %s]\n", node->a, token_to_string(node->a->node_class));
			break;
		}
		// BINARY NODES
		case WHILE:
		case IF: {
			printf("Condition: %p [node %s]\n", node->a, token_to_string(node->a->node_class));
			printf("Tree: %p", node->b);
			if(node->b != NULL)
				printf(" [node %s]", token_to_string(node->b->node_class));
			printf("\n");
			break;
		}
		case LT:
		case GT:
		case EQUALS:
		case LET:
		case GET:
		case STAR:
		case SLASH_FORWARD:
		case PLUS:
		case MINUS:
		case ASSIGN: {
			printf("Left: %p [node %s]\n", node->a, token_to_string(node->a->node_class));
			printf("Right: %p [node %s]\n", node->b, token_to_string(node->b->node_class));
			break;
		}
		// TERNARY NODES
		case IFELSE: {
			printf("Condition: %p [node %s]\n", node->a, token_to_string(node->a->node_class));
			
			printf("Tree: %p", node->b);
			if(node->b != NULL)
				printf(" [node %s]", token_to_string(node->b->node_class));
			printf("\n");
			
			printf("Else: %p", node->c);
			if(node->c != NULL)
				printf(" [node %s]", token_to_string(node->c->node_class));
			printf("\n");
			break;
		}
		// VAR NODE
		case VAR: {
			if(debug_enabled)
				printf("Variable\n");
			
			printf("\tIdentifier: %s\n", (char *)node->a);
			printf("\tType: %s\n", type_to_string(node->type));
			break;
		}
		// CONST NODE
		case CONST: {
			switch(node->type) {
				case TYPE_INT:
					printf("Value: %d\n", ((value_t *)node->a)->int_val);
					break;
				case TYPE_FLOAT:
					printf("Value: %f\n", ((value_t *)node->a)->float_val);
					break;
				default:
					printf("Value: %s\n", "???");
					break;
			}
			
			break;
		}
		case CALL: {
			printf("Function: [%p]\n", node->a);
			printf("	Identifier: %s\n", ((fun_symbol_t *)node->a)->name);
			printf("Args: %p", node->b);
			if(node->b != NULL)
				printf(" [node %s]", token_to_string(node->b->node_class));
			printf("\n");
			break;
		}
		default:
			printf("Can't print node: unexpected node class [%s]\n", token_to_string(node->node_class));
			break;
	}
}

void print_tree(node_t *tree) {
	if(tree == NULL) {
		if(debug_enabled)
			printf("NULL TREE\n");
		return;
	}
	
	if(tree->node_class != SEMICOLON) {
		printf("Can't print non-semicolon tree");
	}
	
	// Traverse nodes, printing
	node_t *curr_node = tree;
	printf("TREE\n");
	printf("------------------------------------------\n");
	while(curr_node != NULL) {
		print_node(curr_node);
		printf("------------------------------------------\n");
		curr_node = curr_node->b; // Print next
	}
}

void type_check(node_t *a, node_t *b) {
	if(a->type != b->type) {
		if(debug_enabled) {
			printf("TYPE ERROR *******************************\n");
			printf("******************************************\n");
			print_node(a);
			printf("******************************************\n");
			print_node(b);
			printf("******************************************\n");
			printf("******************************************\n");
			printf("%s != %s\n", type_to_string(a->type), type_to_string(b->type));
		}
		
		exit(yyerror("unmatching types"));
	}
}

node_t *create_node(const int node_class, node_t *a, node_t *b, node_t *c, int type) {
	node_t *node = (node_t *)malloc(sizeof(node_t));
	
	node->node_class = node_class;
	if(node_class == VAR)
		node->a = (node_t *)strdup((char *)a);
	else
		node->a = a;
	
	node->b = b;
	node->c = c;
	
	// Type-check
	switch(node_class) {
		case CONST:
			node->type = type;
			break;
		case VAR: {
			symbol_t *var = lookup_symbol_scoped((char *)node->a);
			assert(var != NULL); // if this line fails, we are trying to use a variable not defined in active scope
			node->type = var->type;
			break;
		}
		case TILDE:
			node->type = node->a->type;
			break;
		case LT:
		case GT:
		case EQUALS:
		case LET:
		case GET:
			type_check(a, b);
			node->type = TYPE_INT;
			break;
		case STAR:
		case SLASH_FORWARD:
		case PLUS:
		case MINUS:
		case ASSIGN:
			type_check(a, b);
			node->type = a->type; // If this is reached, types are guaranteed to be the same
			break;
		case RETURN:
			assert(node->a != NULL); // we must be returning something
			node->type = node->a->type;
			
			// Type check
			if(scope == NULL) // we are parsing main
				yywarning("using return inside main");
			else { // we are parsing a function
				if(node->type != scope->type) {
					char *err_msg;
					if(asprintf(&err_msg, "unmatching return type %s, expected %s", type_to_string(node->type), type_to_string(scope->type)) < 0) {
						printf("Failed while writing error msg\n");
						exit(0);
					}
					exit(yyerror(err_msg));
				}
			}
			break;
		case CALL: {
			fun_symbol_t *function_symbol = (fun_symbol_t *)node->a;
			node->type = function_symbol->type;
			
			size_t arg_index = 1;
			node_t *curr_arg = node->b;
			while(curr_arg != NULL) {
				if(curr_arg->type != function_symbol->symbols_template[arg_index].type) {
					char *err_msg;
					if(asprintf(&err_msg, "unmatching argument type %s, expected %s", type_to_string(curr_arg->type), type_to_string(function_symbol->symbols_template[arg_index].type)) < 0) {
						printf("Failed while writing error msg\n");
						exit(0);
					}
					exit(yyerror(err_msg));
				}
				curr_arg = curr_arg->b;
				arg_index++;
			}
			break;
		}
		case ARG:
			node->type = node->a->type;
			break;
		default:
			node->type = TYPE_VOID;
			break;
	}
	
	if(debug_enabled) {
		printf("CREATED NODE\n");
		print_node(node);
		printf("\n");
	}
	
	return node;
}

void append_node(node_t *tree, node_t *node) {
	if(debug_enabled)
		printf("Appending [%p] to list starting at [%p]... 1", node, tree);
	
	node_t *last_node = tree;
	int count = 1;
	while(last_node->b != NULL) {
		count++;
		if(debug_enabled)
			printf(" %d", count);
		
		last_node = last_node->b;
	}
	
	// Append node
	last_node->b = node;
	
	if(debug_enabled)
		printf(". Node appended as node %d\n", count + 1);
}


value_t execute(node_t *node) {
	value_t val;
	
	if(node == NULL) {
		if(debug_enabled)
			printf("End of execution branch\n");
		return val;
	}
	
	if(debug_enabled) {
		printf("\nEXECUTION OF NODE...\n");
		printf("Tree entry: %p [%s]\n", node, token_to_string(node->node_class));
	}

	// Traverse nodes, executing
	switch(node->node_class) {
		case SEMICOLON:
			if(!already_returned) {
				execute(node->a); // Execute child sub-tree
				execute(node->b); // Execute next
			}
			break;
		case ASSIGN: {
			symbol_t *symbol = lookup_symbol_exec((char *)node->a->a);
			val = execute(node->b);
			symbol->value = val;
			
			if(debug_enabled) {
				switch(symbol->type) {
					case TYPE_INT:
						printf("Assigned %d to %s\n", val.int_val, symbol->name);
						break;
					case TYPE_FLOAT:
						printf("Assigned %f to %s\n", val.float_val, symbol->name);
						break;
				}
			}
						
			break;
		}
		case IF:
		case IFELSE:
		case WHILE: {
			int expr_type = node->a->type;
			value_t expr_result = execute(node->a);
			if((expr_type == TYPE_INT && expr_result.int_val) || (expr_type == TYPE_FLOAT && expr_result.float_val)) {
				execute(node->b);
				
				if(node->node_class == WHILE)
					execute(node);
			} else {
				execute(node->c);
			}
						
			break;
		}
		case READ: {
			symbol_t *symbol = lookup_symbol_exec((char *)node->a->a);
			assert(symbol != NULL);
			
			if(debug_enabled)
				printf("%s[%s]: ", symbol->name, type_to_string(symbol->type));
			
			switch(symbol->type) {
				case TYPE_INT:
					scanf("%d", &symbol->value.int_val);
					break;
				case TYPE_FLOAT:
					scanf("%f", &symbol->value.float_val);
					break;
				default:
					if(debug_enabled)
						printf("Can't read variable of type: %s\n", type_to_string(symbol->type));

					break;
			}
			
			break;
		}
		case PRINT:
			if(node->a->node_class == VAR && debug_enabled) {
				symbol_t *symbol = lookup_symbol_exec((char *)node->a->a);
				printf("%s: ", symbol->name);
			}
			
			switch(node->a->type) {
				case TYPE_INT:
					printf("%d\n", execute(node->a).int_val);
					break;
				case TYPE_FLOAT:
					printf("%f\n", execute(node->a).float_val);
					break;
				default:
					printf("???\n");
					break;
			}
			break;
		case PLUS:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val + execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.float_val = execute(node->a).float_val + execute(node->b).float_val;
					break;
			}
			break;
		case MINUS:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val - execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.float_val = execute(node->a).float_val - execute(node->b).float_val;
					break;
			}
			break;
		case STAR:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val * execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.float_val = execute(node->a).float_val * execute(node->b).float_val;
					break;
			}
			break;
		case SLASH_FORWARD:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val / execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.float_val = execute(node->a).float_val / execute(node->b).float_val;
					break;
			}
			break;
		case LT:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val < execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.int_val = execute(node->a).float_val < execute(node->b).float_val;
					break;
			}
			break;
		case GT:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val > execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.int_val = execute(node->a).float_val > execute(node->b).float_val;
					break;
			}
			break;
		case EQUALS:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val == execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.int_val = execute(node->a).float_val == execute(node->b).float_val;
					break;
			}
			break;
		case LET:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val <= execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.int_val = execute(node->a).float_val <= execute(node->b).float_val;
					break;
			}
			break;
		case GET:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val >= execute(node->b).int_val;
					break;
				case TYPE_FLOAT:
					val.int_val = execute(node->a).float_val >= execute(node->b).float_val;
					break;
			}
			break;
		case TILDE:
			switch(node->type) {
				case TYPE_INT:
					val.int_val = execute(node->a).int_val * -1;
					break;
				case TYPE_FLOAT:
					val.float_val = execute(node->a).float_val * -1;
					break;
			}
			break;
		case CONST:
			val = *((value_t *)node->a);
			break;
		case VAR:
			val = lookup_symbol_exec((char *)node->a)->value;
			break;
		case ARG:
			val = execute(node->a);
			break;
		case RETURN:
			already_returned = 1;
			if(active_table == NULL) // we're executing main, just exit
				exit(0);
				
			// we're returning from a function
			active_table[0].value = execute(node->a); // save return to _return symbol
			break;
		case CALL: {
			fun_symbol_t *function_symbol = (fun_symbol_t *)node->a;
			
			// Copy function exec environment
			symbol_t *table = calloc(TABLE_SIZE, sizeof(symbol_t));
			memcpy(table, function_symbol->symbols_template, sizeof(symbol_t) * TABLE_SIZE);
			
			// Save previous scope
			symbol_t *prev_active_table = active_table;
			size_t prev_active_next = active_next;
			int prev_returned = already_returned;
			
			// Copy params
			size_t arg_index = 1;
			node_t *curr_arg = node->b;
			while(curr_arg != NULL) {
				table[arg_index].value = execute(curr_arg);
				
				if(debug_enabled) {
					switch(table[arg_index].type) {
						case TYPE_INT:
							printf("Copied argument: %d\n", table[arg_index].value.int_val);
							break;
						case TYPE_FLOAT:
							printf("Copied argument: %f\n", table[arg_index].value.float_val);
							break;
					}
				}
				
				curr_arg = curr_arg->b;
				arg_index++;
			}
			
			// Load new scope table
			active_table = table;
			active_next = function_symbol->symbols_next;
			already_returned = 0;
			
			if(debug_enabled) {
				printf("Table for %s is active\n", function_symbol->name);
				print_symbol_table(active_table, active_next);
			}
			
			// Execute function call
			node_t *tree = function_symbol->tree;
			
			if(debug_enabled)
				printf("Function tree: %p\n", tree);
			
			execute(tree); // should update _return in function symbols
			val = active_table[0].value;
			
			// Restore previous scope
			active_table = prev_active_table;
			active_next = prev_active_next;
			already_returned = prev_returned;
			
			if(debug_enabled) {
				printf("Restored previous table\n");
				print_symbol_table(active_table, active_next);
			}
			
			break;
		}
		default:
			printf("NOT IMPLEMENTED\n");
			break;
	}
	
	return val;
}
