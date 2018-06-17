/*
|--------------------------------------------------------------------------
| Token Module
|--------------------------------------------------------------------------
|
| This module is responsible for managing the token data structure.
|
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "map.h"

#define TOKEN_ARRAY_SIZE 100 

/**
  * Global vars.
  *
  * currentIndex is the index of the current token.
  * currentNode is the current token.
  * fp is the pointer to the out file (lexical analysis file).
  */
int currentIndex = 0;
int backTokenCounter = 0; // This variable counts how many back_token() operations called in a row.
Node *currentNode = NULL;
FILE *lexicalFile_Ptr, *syntacticFile_Ptr, *HTMLlexicalFile_Ptr, *HTMLsyntacticFile_Ptr, *semanticFile_Ptr, *HTMLsemanticFile_Ptr; 

/**
  * Extern vars.
  *
  * yylex() is created inside lex.yy.c by flex.
  * lineNumber is the variable counting the lines while scanning the input.
  * yytext is created inside lex.yy.c by flex.
  */
extern int yylex(), lineNumber;
extern char *yytext;

/**
  * Tokens types array.
  *
  * here are the names of the tokens that are recognized by the given grammer. 
  * the array of names is used for accessing the lexeme by its index (type).
  */
char *names[] = {
    NULL,
    "KEYWORD_ARRAY",
    "KEYWORD_BEGIN",
    "KEYWORD_BLOCK",
    "KEYWORD_DEFAULT",
    "KEYWORD_DO",
    "KEYWORD_END_FOR",
    "KEYWORD_END_WHEN",
    "KEYWORD_END",
    "KEYWORD_FOR",
    "KEYWORD_FREE",
    "KEYWORD_INTEGER",
    "KEYWORD_IS",
    "KEYWORD_MALLOC",
    "KEYWORD_OF",
    "KEYWORD_REAL",
    "KEYWORD_SIZE_OF",
    "KEYWORD_TYPE",
    "KEYWORD_WHEN",
    "OP_PLUS_PLUS",
    "ADDRESS_OF",
    "POINTER",
    "AR_OP_ADD",
    "AR_OP_SUB",
    "AR_OP_POW",
    "AR_OP_MUL",
    "AR_OP_DIV",
    "REL_OP_EQUAL",
    "REL_OP_NOT_EQUAL",
    "REL_OP_GREATER_OR_EQUAL",
    "REL_OP_LESS_OR_EQUAL",
    "REL_OP_GREATER",
    "REL_OP_LESS",
    "ASSIGNMENT",
    "REAL_NUM",
    "INT_NUM",
    "SEPARATOR_COLON",
    "SEPARATOR_SEMECOLON",
    "SEPARATOR_PARENTHESES_OPEN",
    "SEPARATOR_PARENTHESES_CLOSE",
    "SEPARATOR_BRACKETS_OPEN",
    "SEPARATOR_BRACKETS_CLOSE",
    "ID",
    "END_OF_FILE",
};
char **tokenNames = names; // global pointer to names


/**
  * Get next token (from file or memory).
  */
Token *next_token() {
    Token *token;	
    int tokenType;
    char *lexeme;

    if (backTokenCounter) { // there are memory saved tokens
        backTokenCounter--;

        if (currentIndex == TOKEN_ARRAY_SIZE - 1) {
            currentIndex = 0;
            currentNode = currentNode->next;
        } else {
            currentIndex++;
        }

        print_token_to_syntactic_file(
            "<span style=\'color: #3a9ad9;\'>**next_token** (from memory) Token: '%s', Line: %d, Lexeme: '%s', TokenCounter: %d, CurrentIndex: %d.</span><br>", 
            HTMLsyntacticFile_Ptr
            );

        return &(currentNode->tokensArray[currentIndex]);

    } else { // no memory saved tokens
        tokenType = yylex();
        lexeme = strdup(yytext);
        token = create_and_store_token(tokenType, lexeme, lineNumber);

        print_token_to_lexical_file(currentNode->tokensArray[currentIndex]);

        print_token_to_syntactic_file(
            "<span style=\'color: #3a9ad9;\'>**next_token** Token: '%s', Line: %d, Lexeme: '%s', TokenCounter: %d, CurrentIndex: %d.</span><br>", 
            HTMLsyntacticFile_Ptr
            );

        return token;
    }
}


/**
  * Get previous token (1 step backwards in memory).
  */
Token *back_token() { 
    backTokenCounter++;

    if (currentIndex) {
        currentIndex--;
    } else {
        currentIndex = TOKEN_ARRAY_SIZE - 1;
        currentNode = currentNode->prev;
    }

    print_token_to_syntactic_file(
        "<span style=\'color: #b027a4;\'>**back_token** Token: '%s', Line: %d, Lexeme: '%s', TokenCounter: %d, CurrentIndex: %d.</span><br>", 
        HTMLsyntacticFile_Ptr
        );

    return &(currentNode->tokensArray[currentIndex]);
}


/**
  * check if actual token is same as expected token.
  */
Token* match(int token_kind) {
    Token *t = next_token();

    if (t->kind != token_kind) {
        fprintf(syntacticFile_Ptr, "Expected token '%s' at line: %d,\n", names[token_kind],  t->lineNumber);
        fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: red;\'>Expected token '%s' at line: %d,</span><br>",  names[token_kind], t->lineNumber);
        fprintf(syntacticFile_Ptr, "Actual token '%s', lexeme: '%s'.\n", names[t->kind], t->lexeme);   
        fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: red;\'>Actual token '%s', lexeme: '%s'.</span><br>", names[t->kind], t->lexeme);
    } else {
        fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: #354458;\'>**match** Token: '%s', Lexeme: '%s'.</span><br>", names[t->kind], t->lexeme); 
    }
	return t;
}

Token* matchWithBooleanResult(int token_kind, int* result) {
    Token *t = next_token();

    if (t->kind != token_kind) {
        fprintf(syntacticFile_Ptr, "Expected token '%s' at line: %d,\n", names[token_kind],  t->lineNumber);
        fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: red;\'>Expected token '%s' at line: %d,</span><br>",  names[token_kind], t->lineNumber);
        fprintf(syntacticFile_Ptr, "Actual token '%s', lexeme: '%s'.\n", names[t->kind], t->lexeme);   
        fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: red;\'>Actual token '%s', lexeme: '%s'.</span><br>", names[t->kind], t->lexeme);
		*result = 0;
    } else {
        fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: #354458;\'>**match** Token: '%s', Lexeme: '%s'.</span><br>", names[t->kind], t->lexeme); 
		*result = 1;
    }

	return t;
}



Token *create_and_store_token(int kind, char *lexeme, int numOfLine) {
    // case 1: there is still no tokens in the storage.
    if (currentNode == NULL) {
        currentNode = (Node*)malloc(sizeof(Node));

        if (currentNode == NULL) {
            fprintf(yyout, "\nUnable to allocate memory! \n");
            exit(0);
        }
        currentNode->tokensArray = (Token*)malloc(sizeof(Token)*TOKEN_ARRAY_SIZE);
        if (currentNode->tokensArray == NULL) {
            fprintf(yyout, "\nUnable to allocate memory! \n");
            exit(0);
        }
        currentNode->prev = NULL;
        currentNode->next = NULL;
    }

    // case 2: at least one token exsits in the storage.
    else {
        // the array (the current node) is full, need to allocate a new node
        if (currentIndex == TOKEN_ARRAY_SIZE - 1) {
            currentIndex = 0;
            currentNode->next = (Node*)malloc(sizeof(Node));

            if(currentNode == NULL) {
                fprintf(yyout, "\nUnable to allocate memory! \n");
                exit(0);
            }
            currentNode->next->prev = currentNode;
            currentNode = currentNode->next;
            currentNode->tokensArray = (Token*)malloc(sizeof(Token)*TOKEN_ARRAY_SIZE);

            if (currentNode->tokensArray == NULL) {
                fprintf(yyout, "\nUnable to allocate memory! \n");
                exit(0);
            }
            currentNode->next = NULL;
        }

        // the array (the current node) is not full
        else {
            currentIndex++;
        }
    }

    currentNode->tokensArray[currentIndex].kind = kind;
    currentNode->tokensArray[currentIndex].lexeme = (char*)malloc(sizeof(char)*(strlen(lexeme)+1));
    strcpy(currentNode->tokensArray[currentIndex].lexeme, lexeme);
    currentNode->tokensArray[currentIndex].lineNumber = numOfLine;
    return &(currentNode->tokensArray[currentIndex]);
}


/** 
  * Free the allocated memory of the token data structure.
  * 
  * cleaning each node from memory as well as each dynamic array.
  */
void cleanMemory() {
    Node *tmpNode;
    Node *headOfList = currentNode;

    while (currentNode != NULL) {
        tmpNode = currentNode;
        currentNode = currentNode->prev;
        free(tmpNode->tokensArray);
        free(tmpNode);
    }

    headOfList = NULL;

    fclose(lexicalFile_Ptr);
    fclose(HTMLlexicalFile_Ptr);
    fclose(syntacticFile_Ptr);
    fclose(HTMLsyntacticFile_Ptr);
	fclose(semanticFile_Ptr);
    fclose(HTMLsemanticFile_Ptr);

	cleanTypeTable();
}


/** 
  * Print token to file.
  * 
  * printing the kind of the token, its lexeme and its location in the input file.
  * can also print to screen (by uncomment the print_token(t) line below). 
  */
void print_token_to_lexical_file(Token token) {
    fprintf(lexicalFile_Ptr, 
        "Token of kind '%s' was found at line: %d, lexeme: '%s'.\n",
        names[token.kind],													
        token.lineNumber,
        token.lexeme
        );	

    fprintf(HTMLlexicalFile_Ptr, 
        "<span style=\'color: #3a9ad9;\'>Token of kind '%s' was found at line: %d, lexeme: '%s'.</span><br>\n",
        names[token.kind],
        token.lineNumber,
        token.lexeme
        );
}


void print_token_to_syntactic_file(char *format, FILE *fp) {
    fprintf(fp,      
        format,
        names[currentNode->tokensArray[currentIndex].kind],
        currentNode->tokensArray[currentIndex].lineNumber,
        currentNode->tokensArray[currentIndex].lexeme, 
        backTokenCounter,
        currentIndex
        );
}