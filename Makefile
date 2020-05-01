OUT             = compiler
BISON_Y         = parser.y
BISON_C         = parser.c
BISON_H         = parser.h
LEX_L           = lexer.lex
LEX_C           = lexer.yy.c
SRC_DIR 		= src
BIN_DIR 		= bin
CPPFLAG         = -ll



all: $(BIN_DIR)/$(OUT)

compiler: $(BIN_DIR)/$(OUT) clean

$(SRC_DIR)/$(BISON_C): $(SRC_DIR)/$(BISON_Y)
	bison -d -o $(SRC_DIR)/$(BISON_C) $(SRC_DIR)/$(BISON_Y)
	
$(SRC_DIR)/$(LEX_C): $(SRC_DIR)/$(LEX_L)
	flex -o $(SRC_DIR)/$(LEX_C) $(SRC_DIR)/$(LEX_L)

$(BIN_DIR)/$(OUT): $(SRC_DIR)/$(LEX_C) $(SRC_DIR)/$(BISON_C)
	gcc -o $(BIN_DIR)/$(OUT) $(SRC_DIR)/$(LEX_C) $(SRC_DIR)/$(BISON_C) $(CPPFLAG)



clean:
ifeq ($(OS),Windows_NT)
	del /s /q $(SRC_DIR)\$(BISON_C) $(SRC_DIR)\$(BISON_H) $(SRC_DIR)\$(LEX_C)
else
	rm -f $(SRC_DIR)/$(BISON_C) $(SRC_DIR)/$(BISON_H) $(SRC_DIR)/$(LEX_C)
endif

veryclean: clean
ifeq ($(OS),Windows_NT)
	del /s /q $(BIN_DIR)\$(OUT)
else
	rm -f $(BIN_DIR)/$(OUT)
endif

