/*
|--------------------------------------------------------------------------
| Utility Module
|--------------------------------------------------------------------------
|
| This module contains extra utility functions for the lexical & syntactic analyser.
|
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
 
/**
  * Extern vars.
  *
  * yylex() is created inside lex.yy.c by flex.
  * lineNumber is the variable counting the lines while scanning the input.
  * yytext is created inside lex.yy.c by flex.
  */
extern Node *currentNode;
extern int yylex(), lineNumber, currentIndex; 
extern char **tokenNames;  // global pointer to names
extern char *yytext, *in, *outLexical, *outLexical_HTML, *outSyntactic, *outSyntactic_HTML, *outSemantic, *outSemantic_HTML;
extern FILE *lexicalFile_Ptr, *HTMLlexicalFile_Ptr, *syntacticFile_Ptr, *HTMLsyntacticFile_Ptr, *semanticFile_Ptr, *HTMLsemanticFile_Ptr;


/* init html file */
void initHTMLsemanticFile(FILE *HTMLsemanticFile_Ptr) {
    fprintf(HTMLsemanticFile_Ptr, 
        "<html lang=\'en\' dir=\'ltr\'>\
        <head>\
            <meta charset=\'utf-8\'>\
            <title></title>\
            <link rel=\'stylesheet\' href=\'https://stackpath.bootstrapcdn.com/bootstrap/4.1.1/css/bootstrap.min.css\'>\
            <style>\
                .table td, .table th {padding: .2rem .4rem;}\
            </style>\
        </head>\
        <body>\
            <div class=\'container\'>"
    );
}

/* printing to html method */
void printToSemanticHTML(char *msg) {
	fprintf(syntacticFile_Ptr, "{ %s }\n", msg);
    fprintf(HTMLsemanticFile_Ptr, "<br><br><span style=\'color: #19bd18; font-weight: 700;\'>{ %s }</span><br>", msg);
}

void init_parser(int fileId) {
    lineNumber = 1;
    currentIndex = 0;

    if (fileId == 1) { // 1st test file setup.
        in = "C:\\temp\\test1.txt";
        outLexical = "C:\\temp\\test1_039525910_203925680_lex.txt";
        outLexical_HTML = "C:\\temp\\test1_039525910_203925680_lex.html";
        outSyntactic = "C:\\temp\\test1_039525910_203925680_syntactic.txt"; 
        outSyntactic_HTML = "C:\\temp\\test1_039525910_203925680_syntactic.html";
	    outSemantic = "C:\\temp\\test1_039525910_203925680_semantic.txt"; 
        outSemantic_HTML = "C:\\temp\\test1_039525910_203925680_semantic.html";

    } else if (fileId == 2) { // 2nd test file setup.

        in = "C:\\temp\\test2.txt";
        outLexical = "C:\\temp\\test2_039525910_203925680_lex.txt"; 
        outLexical_HTML = "C:\\temp\\test2_039525910_203925680_lex.html";
        outSyntactic = "C:\\temp\\test2_039525910_203925680_syntactic.txt"; 
        outSyntactic_HTML = "C:\\temp\\test2_039525910_203925680_syntactic.html";
	    outSemantic = "C:\\temp\\test2_039525910_203925680_semantic.txt"; 
        outSemantic_HTML = "C:\\temp\\test2_039525910_203925680_semantic.html";
    }
    yyin = fopen(in,"r");
    lexicalFile_Ptr = fopen(outLexical, "w");
    HTMLlexicalFile_Ptr = fopen(outLexical_HTML, "w");
    syntacticFile_Ptr = fopen(outSyntactic, "w");
    HTMLsyntacticFile_Ptr = fopen(outSyntactic_HTML, "w");
	semanticFile_Ptr = fopen(outSemantic, "w");
    HTMLsemanticFile_Ptr = fopen(outSemantic_HTML, "w");
	initHTMLsemanticFile(HTMLsemanticFile_Ptr);
}

void print_rule_to_file(char *msg) {
    fprintf(syntacticFile_Ptr, "{ %s }\n", msg);
    fprintf(HTMLsyntacticFile_Ptr, "<br><br><span style=\'color: #19bd18; font-weight: 700;\'>{ %s }</span><br>", msg);
}



/**
  * Handling a syntactic error.
  *
  * gets pointer to follow_X function, expected token & current token.
  * eg. errorHandler(follow_DEFINITIONS_TAG, token, "'ID', 'KEYWORD_TYPE','KEYWORD_BEGIN'", 0);
  */
void errorHandler(int (*followMethod)(Token *t), Token *token, char *expectedToken, int isOneTokenExpected) {	
    if (isOneTokenExpected) {
        fprintf(syntacticFile_Ptr, "Expected token '%s' at line: %d,\n", expectedToken, token->lineNumber);
        fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: red;\'>Expected token '%s' at line: %d,</span><br>", expectedToken, token->lineNumber);
    } else {
        fprintf(syntacticFile_Ptr, "Expected one of tokens %s at line: %d,\n", expectedToken, token->lineNumber);
        fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: red;\'>Expected one of tokens %s at line: %d,</span><br>", expectedToken, token->lineNumber);
    }
    fprintf(syntacticFile_Ptr, "Actual token '%s', lexeme: '%s'.\n", tokenNames[token->kind], token->lexeme);   
    fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: red;\'>Actual token '%s', lexeme: '%s'.</span><br>", tokenNames[token->kind], token->lexeme);

	/* Now iterating and skipping tokens until we find a token which belongs to the follow of the grammer Variable */
    while (!followMethod(token) && token->kind != END_OF_FILE){
        token = next_token();
		fprintf(HTMLsyntacticFile_Ptr, "<span style=\'color: red;\'>Skipping token '%s'.</span><br>", tokenNames[token->kind]);
    }

	if (followMethod(token)) {
		back_token(); /* if the token belongs to the follow of the variable we need to go back_token */
	}
}

/**
  * Handling a semantic error.
  */

void errorHandlerSemantic(Token *token, char* msg) { 
	fprintf(semanticFile_Ptr, msg);
	printToSemanticHTML(msg);
}

