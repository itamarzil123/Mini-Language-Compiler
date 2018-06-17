%{
    #include <stdlib.h>
	#include "token.h"
	
	char *outSyntactic, *in, *outLexical, *outSyntactic_HTML, *outLexical_HTML;
	extern FILE *lexicalFile_Ptr, *HTMLlexicalFile_Ptr, *syntacticFile_Ptr, *HTMLsyntacticFile_Ptr;

	int lineNumber = 1;
	extern Node *currentNode;
    void yyerror(int lineNumber, char* text);
%}

%%
\n				lineNumber++;

"array"			return KEYWORD_ARRAY;
"begin"			return KEYWORD_BEGIN;
"block"			return KEYWORD_BLOCK;
"default"		return KEYWORD_DEFAULT;
"do"			return KEYWORD_DO;
"end_for"		return KEYWORD_END_FOR;
"end_when"		return KEYWORD_END_WHEN;
"end"			return KEYWORD_END;
"for"			return KEYWORD_FOR;
"free"			return KEYWORD_FREE;
"integer"		return KEYWORD_INTEGER;
"is"			return KEYWORD_IS;
"malloc"		return KEYWORD_MALLOC;
"of"			return KEYWORD_OF;
"real"			return KEYWORD_REAL;
"size_of"		return KEYWORD_SIZE_OF;
"type"			return KEYWORD_TYPE;
"when"			return KEYWORD_WHEN;

\+\+			return OP_PLUS_PLUS;
\&				return ADDRESS_OF;
\^				return POINTER;

"--".*          ;

\+				return AR_OP_ADD;
\-				return AR_OP_SUB;
\*\*			return AR_OP_POW;
\*				return AR_OP_MUL;
\/				return AR_OP_DIV;

\=\=			return REL_OP_EQUAL;
\!\=			return REL_OP_NOT_EQUAL;
\>\=			return REL_OP_GREATER_OR_EQUAL;
\<\=			return REL_OP_LESS_OR_EQUAL;
\>				return REL_OP_GREATER;
\<				return REL_OP_LESS;

\=				return ASSIGNMENT;

[0-9]+\.[0-9]+	return REAL_NUM;
0|[1-9][0-9]*	return INT_NUM;

\:				return SEPARATOR_COLON;
\;				return SEPARATOR_SEMECOLON;
\(				return SEPARATOR_PARENTHESES_OPEN;
\)				return SEPARATOR_PARENTHESES_CLOSE;
\[				return SEPARATOR_BRACKETS_OPEN;
\]				return SEPARATOR_BRACKETS_CLOSE;

[a-zA-Z][a-zA-Z0-9]*(\_[a-zA-Z0-9]+)*[a-zA-Z0-9]* 	return ID;

[ \t\n]			;
<<EOF>>			return END_OF_FILE;

.				yyerror(lineNumber, yytext);
%%

int yywarp(void) {
    return 1;
}

void yyerror(int errorLine, char *errorText) {
    fprintf(lexicalFile_Ptr, 
        "The character '%s' at line: %d does not begin any legal token in the language.\n",
        errorText,
        errorLine
        );
    fprintf(HTMLlexicalFile_Ptr, 
        "<span style=\'color: red;\'>The character '%s' at line: %d does not begin any legal token in the language.</span><br>\n",
        errorText,
        errorLine
        );
}

void parse_file(int n){
    init_parser(n);
    parser(); 
    cleanMemory();
    printf(
        "Start prosssing '%s' ...\nPassed lexical analysis successfully.\nPassed syntactic analysis successfully.\n\n",
        in
        );
}

int main() {
    parse_file(1);
    yyrestart(yyin);
    parse_file(2);

    return 0;
}