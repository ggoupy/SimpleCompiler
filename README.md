# Parser
This is the parser for the final project (with the language defined in `Homework 7`) of the *Compilers Analysis and Design* course for Tecnologico de Monterrey, Puebla. It is implemented using [Lex](https://github.com/westes/flex) to generate a token stream from the input file, which is then parsed using [Bison](https://www.gnu.org/software/bison/).


## Usage
You need to compile the lexer and parser before using them.

Using Makefile:
```bash
> make
```
To compile and remove files created:
```bash
> make compiler
```
To remove files created:
```bash
> make clean
```
To remove binary file and files created:
```bash
> make veryclean
```

Or you can compile files by your own:
```bash
> flex src/lexer.lex
> bison --defines=parser.h src/parser.y
> gcc src/lex.yy.c src/parser.c -ll -o bin/compiler
```

Note you need `flex`, `bison` and `gcc` or other C compiler installed. You can use any other C compiler and the appropriate flex library flag for your environment.

To run the program:
```bash
> bin/compiler <filename> [--debug]
```

You can use the `--debug` flag to get verbose output during parsing.


## Input file
Please note the parser will ignore non-standard new lines (any other than `\n`). You also need to specify an existing file. Tab is considered one single character for column counting purposes.


## Grammar
The homework grammar was used almost directly with Bison, with only a few minor changes:
- arg_lst is now `arg_lst` -> `arg`, `arg_lst` | `arg`
- Added rule `arg` -> `expr`
- stmt assign rule changed to `stmt` -> `id` <- `expr` (using `id` rule)
- stmt read rule changed to `stmt` -> read `id` (using `id` rule)
- factor id rule changed to `factor` -> `id` (using `id` rule)
- Added rule `id` -> `<ID>`
- fun_dec is now `fun_dec` -> fun `fun_id`(`opt_params`): `<TYPE>` `opt_decls` begin `opt_stmts` end (using `fun_id` rule)
- Added rule `fun_id` -> `<ID>`


## Symbol table
Please note the symbol table has a limited static size. The parser will expectedly fail when this bound is reached and a new variable is attempted to be assigned. You can change the size by setting the `TABLE_SIZE` macro in `parser.y`.

## Functions
This implementation supports the following functions features:

- Function declaration
- Function call
- Function returns
- Arguments type-check
- Call-length check (number of parameters)
- Recursion
- Nested calls

See `test/sample2.txt`, `test/sample3.txt` and `test/sample4.txt` for a demonstration of these capabilities.
See `test/sample5.txt` and `test/sample6.txt` for error reporting on function calls.
Please note the implicit main function will just exit on `return`, ignoring subsequent statements.
