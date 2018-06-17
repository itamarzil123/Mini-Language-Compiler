/*
|--------------------------------------------------------------------------
| Token Header Module: This module contains the logics of managing tokens.
|--------------------------------------------------------------------------
|
*/

#ifndef TOKEN_H
#define TOKEN_H

extern FILE *yyin, *yyout;

/* keywords */
#define KEYWORD_ARRAY 1
#define KEYWORD_BEGIN 2
#define KEYWORD_BLOCK 3
#define KEYWORD_DEFAULT 4
#define KEYWORD_DO 5
#define KEYWORD_END_FOR 6
#define KEYWORD_END_WHEN 7
#define KEYWORD_END 8
#define KEYWORD_FOR 9
#define KEYWORD_FREE 10
#define KEYWORD_INTEGER 11
#define KEYWORD_IS 12
#define KEYWORD_MALLOC 13
#define KEYWORD_OF 14
#define KEYWORD_REAL 15
#define KEYWORD_SIZE_OF 16
#define KEYWORD_TYPE 17
#define KEYWORD_WHEN 18

/* unary operations */
#define OP_PLUS_PLUS 19       // ++
#define ADDRESS_OF 20         // &
#define POINTER 21            // ^

/* arithmetic operations */
#define AR_OP_ADD 22          // +
#define AR_OP_SUB 23          // -
#define AR_OP_POW 24          // **
#define AR_OP_MUL 25          // *
#define AR_OP_DIV 26          // /

/* relation operations */
#define REL_OP_EQUAL 27             // ==
#define REL_OP_NOT_EQUAL 28         // !=
#define REL_OP_GREATER_OR_EQUAL 29  // >=
#define REL_OP_LESS_OR_EQUAL 30     // <=
#define REL_OP_GREATER 31           // >
#define REL_OP_LESS 32              // <

/* assignment */
#define ASSIGNMENT 33         // =

/* numbers */
#define REAL_NUM 34           // eg. 123.456
#define INT_NUM 35            // eg. 123

/* separators */
#define SEPARATOR_COLON 36              // :
#define SEPARATOR_SEMECOLON 37          // ;
#define SEPARATOR_PARENTHESES_OPEN 38   // (
#define SEPARATOR_PARENTHESES_CLOSE 39  // )
#define SEPARATOR_BRACKETS_OPEN 40      // [
#define SEPARATOR_BRACKETS_CLOSE 41     // ]

/* id */
#define ID 42                 // string of alphanumeric letters

#define END_OF_FILE 43                 

typedef struct Token {
    int kind;
    char *lexeme;
    int lineNumber;
} Token;

typedef struct Node {
    Token *tokensArray;
    struct Node *prev;
    struct Node *next;
} Node;

Token *next_token();
Token *back_token();
Token* match(int token_kind);
Token* matchWithBooleanResult(int token_kind, int* result);

Token *create_and_store_token(int kind, char *lexeme, int numOfLine);
void cleanMemory();
void print_token_to_lexical_file(Token token);
void print_token_to_syntactic_file(char *format, FILE *fp);

#endif