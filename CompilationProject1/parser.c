/*
|--------------------------------------------------------------------------
| Parser Module
|--------------------------------------------------------------------------
|
| This module contains parse_X() & follow(X) functions for each of the grammer's variables
| lex.yy.c main contains a call for parse() method to start the parsing process of the given input.
|
| note: next_token, back_token & match methods are implemented in token.c
|
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "token.h"
#include "file_utility.h"
#include "symbol_table_utility.h"

/* an indicator for a variable that has not been defined (used inside 
SSION) */
#define UNDEFINED_ID -999 

/* an indicator for a type that has already been defined in this scope(used inside parse_EXPRESSION) */
#define UNDEFINED_TYPE -996 

/* an indicator for reaching a default switch case inside a parser. */
#define DEFAULT_ERROR -998

/*an indicator for using address operator '&' on a declaration. */
#define ADDRESS_INDICATOR -997



/**
* Extern vars.
*
* fp is the file pointer used for IO.
* lineNumber is the variable counting the lines while scanning the input.
* out,in, are source, target file paths.
*/
extern char *in;
extern int lineNumber;
extern FILE *lexicalFile_Ptr, *syntacticFile_Ptr, *semanticFile_Ptr;


/* this method does a  matching check on re_op operators. It is used only in parse_COMMAND for matching rel ops.*/
void match_rel_op();



/* * * * * * * * * * * */
/*Start - Global Vars. */
/* * * * * * * * * * * */

/* a token variable used inside the parser functions. */
Token *token; // remains without the Global_token for convenience.

/* a Global Array Of Scopes. (Array of symbol tables stored in a hirarchy fasion). */
ScopeArray Global_scopeArrayOfSymbolTables;

/* a Global Scope Element.  each time a new symbol table is created, Global_newScopeElement will also be used. */
ScopeElement Global_newScopeElement; 
ScopeElement* Global_newScopeElementPtr; 
SymbolTable* Global_currentSymbolTablePtr;
struct StackNode* Global_currentScopeStack = NULL; // ordinary stack helps us track the current scope
Entry Global_tempEntry; // value
char* Global_tempSymbolTableKey; // key
SymbolTable Global_tempSymbolTable;
TypeStringTable Global_typeStringTable;

/* * * * * * * * * * * /
/* End - Global Vars.* /
/* * * * * * * * * * * /






/**
* Parsing Methods ( Declarations )
*/
void parser();
void parse_PROGRAM();
void parse_BLOCK();
void parse_DEFINITIONS();
void parse_DEFINITIONS_TAG();
void parse_DEFINITION();
void parse_VAR_DEFINITION();
void parse_VAR_DEFINITION_TAG(int isSavedEntryKeyValid);
void parse_TYPE_DEFINITION();
void parse_TYPE_INDICATOR(int isSavedEntryKeyValid);
char* parse_BASIC_TYPE();
void parse_ARRAY_TYPE(int isSavedEntryKeyValid);
void parse_POINTER_TYPE(int isSavedEntryKeyValid);
void parse_POINTER_TYPE_TAG(int isSavedEntryKeyValid);
int parse_SIZE();
void parse_COMMANDS();
void parse_COMMANDS_TAG();
void parse_COMMAND();
Entry parse_RECEIVER();
Entry parse_RECEIVER_TAG(Entry receiverType);
Entry parse_EXPRESSION();
Entry parse_EXPRESSION_TAG(Entry expressionType);

/**
* Follow() Methods ( Declarations )
*/
int follow_PROGRAM(Token *token);
int follow_BLOCK(Token *token);
int follow_DEFINITIONS(Token *token);
int follow_DEFINITIONS_TAG(Token *token);
int follow_DEFINITION(Token *token);
int follow_VAR_DEFINITION(Token *token);
int follow_VAR_DEFINITION_TAG(Token *token);
int follow_TYPE_DEFINITION(Token *token);
int follow_TYPE_INDICATOR(Token *token);
int follow_BASIC_TYPE(Token *token);
int follow_ARRAY_TYPE(Token *token);
int follow_POINTER_TYPE(Token *token);
int follow_POINTER_TYPE_TAG(Token *token);
int follow_SIZE(Token *token);
int follow_COMMANDS(Token *token);
int follow_COMMANDS_TAG(Token *token);
int follow_COMMAND(Token *token);
int follow_RECEIVER(Token *token);
int follow_RECEIVER_TAG(Token *token);
int follow_EXPRESSION(Token *token);
int follow_EXPRESSION_TAG(Token *token);

/**
 * More Helpers
 */
int saveEntryKey(char* key);
int isTypeExistInScopeHirarchy(char* lexeme);
int isTypeExistInThisScope(char* lexeme);
int isValidType(int inputType);
int isDeclaredID(char* Id);
int NotValidReceiverAndExpression(Entry receiverType, Entry expressionType);
int receiverAndExpressionPointersToTheSameType(Entry receiver, Entry expression);
int isIDdeclaredAndTypeNotExistInThisScope(char* name);
int isTypeIntegerOrReal(char *type);
int isNotErrorType(Entry expression);
void handleArithmaticInconsistency(Entry expression1, Entry expression2);
int isFromBasicType(Entry expression);

/**
* Parsing Methods ( Implementations)
*/
void parser() {
    parse_PROGRAM();
    match(END_OF_FILE);
}


void parse_PROGRAM() {
    print_rule_to_file("PROGRAM -> BLOCK");

    /* Initialization Of The Scope Array Of Symbol Tables */
    initScopeArray(&Global_scopeArrayOfSymbolTables, 1);

    /*  Updating the Scope Stack. The parent of the global scope is the bottom of the stack */
    initScopeStack(); 

    /* Initialization of the Type Table */
    initTypeStrTable();

    parse_BLOCK();
}


void parse_BLOCK() {

    print_rule_to_file("BLOCK -> block DEFINITIONS ; begin COMMANDS ; end");
    match(KEYWORD_BLOCK); 

    newScope(); /*  We are entering a new scope. logics is done inside newScope() method which is handling and managing entering into a new scope.  */

    parse_DEFINITIONS();
    match(SEPARATOR_SEMECOLON);
    match(KEYWORD_BEGIN);
    parse_COMMANDS();
    match(SEPARATOR_SEMECOLON);
    match(KEYWORD_END); 

    exitScope();  /*  We are leaving a scope. logics is done inside exitScope() method which is handling and managing leaving a scope.  */
}


void parse_DEFINITIONS() {
    print_rule_to_file("DEFINITIONS -> DEFINITION  DEFINITIONS_TAG");
    parse_DEFINITION();
    parse_DEFINITIONS_TAG();
}


void parse_DEFINITIONS_TAG() {
    token = next_token();

    switch (token->kind) {
    case SEPARATOR_SEMECOLON:
        token = next_token(); 
        switch (token->kind) {
        case ID:
        case KEYWORD_TYPE:
            print_rule_to_file("DEFINITIONS_TAG -> ; DEFINITION  DEFINITIONS_TAG");
            back_token();
            parse_DEFINITION();
            parse_DEFINITIONS_TAG();
            break;

        case KEYWORD_BEGIN:
            print_rule_to_file("DEFINITIONS_TAG -> epsilon");
            back_token();
            back_token();
            break;

        default:
            errorHandler(follow_DEFINITIONS_TAG, token, "'ID', 'KEYWORD_TYPE','KEYWORD_BEGIN'", 0);
            break;
        }
        break;

    default:
        errorHandler(follow_DEFINITIONS_TAG, token, "'SEPARATOR_SEMICOLON'", 1);
        break;
    }
}


void parse_DEFINITION() {
    token = next_token();

    switch (token->kind) {
    case ID:  
        print_rule_to_file("DEFINITION -> VAR_DEFINITION");
        setEntryRole(VARIABLE);
        back_token(); 
		setEntryInheritedData(NULL); // reset pointer safety. (before later initialization if needed).
        parse_VAR_DEFINITION();
        break;

    case KEYWORD_TYPE:
        print_rule_to_file("DEFINITION -> TYPE_DEFINITION");
        setEntryRole(USER_DEFINED_TYPE);
        back_token(); 
		setEntryInheritedData(NULL); // reset pointer safety. (before later initialization if needed).
        parse_TYPE_DEFINITION();
        break;

    default:
        errorHandler(follow_DEFINITION, token, "'ID', 'KEYWORD_TYPE'", 0);
        break;
    }
}


void parse_VAR_DEFINITION() {
    int isSavedEntryKeyValid = 1;
    print_rule_to_file("VAR_DEFINITION -> id : VAR_DEFINITION_TAG");
    token = match(ID);

    /**  If the Key (the ID) already exists or trying to use an already existing
      *   type name in this scope, alert with error. (error handling is built in inside saveEntryKey() method.)  
	  */
    if (!saveEntryKey(token->lexeme)) {
        isSavedEntryKeyValid = 0;
    } 

    /*  Variables do not have a subtype, category and array size.  */
    setEntrySubtype("");
    setEntryCategory(NO_CATEGORY);
    setEntryArraySize(0);

    match(SEPARATOR_COLON);
    parse_VAR_DEFINITION_TAG(isSavedEntryKeyValid);
}  


void parse_VAR_DEFINITION_TAG(int isSavedEntryKeyValid) { 
    Entry* lookupResult;

    token = next_token();

    switch (token->kind) {
    case KEYWORD_INTEGER:
        print_rule_to_file("VAR_DEFINITION_TAG -> BASIC_TYPE");

        /*  Updating the global temp entry according to a var definition of basic type. */
        setEntryType("integer");
        setEntryInheritedData(NULL);
        if (isSavedEntryKeyValid) {
            updateGlobalTempSymbolTable(); 
        }

        back_token(); 
        parse_BASIC_TYPE();
        break;
    case KEYWORD_REAL:
        print_rule_to_file("VAR_DEFINITION_TAG -> BASIC_TYPE");

        /* updating the global temp entry according to a var definition of basic type */
        setEntryType("real");
        setEntryInheritedData(NULL);

        if (isSavedEntryKeyValid) {
            updateGlobalTempSymbolTable(); 
        }

        back_token(); 
        parse_BASIC_TYPE();
        break;

    case ID:
        print_rule_to_file("VAR_DEFINITION_TAG -> type_name");

        /* If the type name does not exist, error alert */
        if (!isTypeExistInScopeHirarchy(token->lexeme)) { 
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' is not defined.\n", token->lineNumber, token->lexeme);
        } else {
            lookupResult = lookupHirarchyAndReturnEntry(token->lexeme);
            setEntryType(token->lexeme); 
            setEntryInheritedData(lookupResult); 

            if (isSavedEntryKeyValid) {
                updateGlobalTempSymbolTable(); 
            }			
        }
        break;

    default:
        errorHandler(follow_VAR_DEFINITION_TAG, token, "'KEYWORD_INTEGER', 'KEYWORD_REAL', 'ID'", 0);
        break;
    }
}


void parse_TYPE_DEFINITION() {
    int isSavedEntryKeyValid;

    print_rule_to_file("TYPE_DEFINITION -> type type_name is TYPE_INDICATOR");
    match(KEYWORD_TYPE);
    token = match(ID);


    /**
    * If the Key (the ID) already exists or the type already exist IN THIS SCOPE, alert with error. 
    *  (error handling logics is built in inside saveEntryKey method. 
    */
    if (!saveEntryKey(token->lexeme)) {  // : change to dependency injection instead of global var
        isSavedEntryKeyValid = 0; 
    } else {
        isSavedEntryKeyValid = 1;
    }

    /*   User-defined-types have no "type"   */
    setEntryType("");

    /*  Adding the new type to the type table.  */
    addToTypeTable(&Global_typeStringTable, token->lexeme);

    match(KEYWORD_IS);	
    parse_TYPE_INDICATOR(isSavedEntryKeyValid);
}


void parse_TYPE_INDICATOR(int isSavedEntryKeyValid) {
    token = next_token();

    switch(token->kind) {
    case KEYWORD_INTEGER: // example: type salary is integer;	
        print_rule_to_file("TYPE_INDICATOR -> BASIC_TYPE");

        /*  Updating the global temp entry according to a type definition of basic type.  */
        setEntryCategory(BASIC_CATEGORY);
        setEntryArraySize(0);
        setEntrySubtype("integer"); 
        setEntryInheritedData(NULL);
        if (isSavedEntryKeyValid) {
            updateGlobalTempSymbolTable(); 
        }
        back_token();
        parse_BASIC_TYPE();
        break;

    case KEYWORD_REAL: // example: type salary is real;
        setEntryType("");
        print_rule_to_file("TYPE_INDICATOR -> BASIC_TYPE");

        /*  Updating the global temp entry according to a type definition of basic type.  */
        setEntryCategory(BASIC_CATEGORY);
        setEntryArraySize(0);
        setEntrySubtype("real"); 
        setEntryInheritedData(NULL);
        if (isSavedEntryKeyValid) {
            updateGlobalTempSymbolTable(); 
        }

        back_token();
        parse_BASIC_TYPE();
        break;

    case KEYWORD_ARRAY: // example: type data_arr is array [10] of integer; 
        print_rule_to_file("TYPE_INDICATOR -> ARRAY_TYPE");

        /*  Updating the global temp entry according to a type array definition.  */
        setEntryCategory(ARRAY_CATEGORY);
        setEntryInheritedData(NULL);
        back_token();
        parse_ARRAY_TYPE(isSavedEntryKeyValid);
        break;

    case POINTER: // example: type ptr_data_arr is ^data_arr;     
        print_rule_to_file("TYPE_INDICATOR -> POINTER_TYPE");

        /*  Updating the global temp entry according to a type definition of pointer type.  */
        setEntryCategory(POINTER_CATEGORY);
        setEntryArraySize(0);

        back_token();
        parse_POINTER_TYPE(isSavedEntryKeyValid);
        break;

    default:
        errorHandler(follow_TYPE_INDICATOR, token, "'KEYWORD_INTEGER', 'KEYWORD_REAL', 'KEYWORD_ARRAY', 'POINTER'", 0);
        break;
    }
}


char* parse_BASIC_TYPE() {
    token = next_token();

    switch(token->kind) {
    case KEYWORD_INTEGER:
        print_rule_to_file("BASIC_TYPE -> integer");
        return "integer";
        break;

    case KEYWORD_REAL:
        print_rule_to_file("BASIC_TYPE -> real");
        return "real";
        break;

    default:
        errorHandler(follow_BASIC_TYPE, token, "'KEYWORD_INTEGER', 'KEYWORD_REAL'", 0);
        return NULL;
        break;
    }
}


void parse_ARRAY_TYPE(int isSavedEntryKeyValid) {
    char* type;
    int isArraySizeValid;

    print_rule_to_file("ARRAY_TYPE -> array [ SIZE ] of BASIC_TYPE");
    match(KEYWORD_ARRAY);
    match(SEPARATOR_BRACKETS_OPEN);
    isArraySizeValid = parse_SIZE(); 	
    match(SEPARATOR_BRACKETS_CLOSE);
    match(KEYWORD_OF);
    type = parse_BASIC_TYPE(); 

    /*   Updating the global temp entry according to a type array definition.  */
    setEntrySubtype(type); 
    if (isSavedEntryKeyValid && isArraySizeValid) {
        updateGlobalTempSymbolTable(); 
    }
}


void parse_POINTER_TYPE(int isSavedEntryKeyValid) {
    print_rule_to_file("POINTER_TYPE -> ^ POINTER_TYPE_TAG");
    match(POINTER);
    parse_POINTER_TYPE_TAG(isSavedEntryKeyValid);
}


void parse_POINTER_TYPE_TAG(int isSavedEntryKeyValid) {
    Entry* lookupResult;

    token = next_token();

    switch(token->kind) {
    case KEYWORD_INTEGER:
        print_rule_to_file("POINTER_TYPE_TAG -> BASIC_TYPE"); 

        /*   Updating the global temp entry according to a type definition of pointer basic type. */
        setEntrySubtype("integer");
        setEntryInheritedData(NULL);
        if (isSavedEntryKeyValid) {
            updateGlobalTempSymbolTable(); 
        }

        back_token();
        parse_BASIC_TYPE();
        break;

    case KEYWORD_REAL:
        print_rule_to_file("POINTER_TYPE_TAG -> BASIC_TYPE"); 

        /*   Updating the global temp entry according to a type definition of pointer basic type. */
        setEntrySubtype("real");
        setEntryInheritedData(NULL);
        if (isSavedEntryKeyValid) {
            updateGlobalTempSymbolTable(); 
        }

        back_token();
        parse_BASIC_TYPE();
        break;

    case ID:
        print_rule_to_file("POINTER_TYPE_TAG -> type_name");

        /*  Making sure the type exists. */  

        if (isTypeExistInScopeHirarchy(token->lexeme)) {
            lookupResult = lookupHirarchyAndReturnEntry(token->lexeme);
            setEntryInheritedData(lookupResult);
            setEntrySubtype(token->lexeme);
            if (isSavedEntryKeyValid) {
                updateGlobalTempSymbolTable(); 
            }

            setEntryType("");
        } else {
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' is not defined.\n", token->lineNumber, token->lexeme);
        }
        break;

    default:
        errorHandler(follow_POINTER_TYPE_TAG, token, "'KEYWORD_INTEGER', 'KEYWORD_REAL', 'ID'", 0);
        break;
    }
}


int parse_SIZE() {
    Token *token;
    char* sizeAsString;
    int sizeAsInteger;
    int result;

    print_rule_to_file("SIZE -> int_num");

    /*   Updating the global temp entry according to the array size. */
    token = matchWithBooleanResult(INT_NUM, &result);

    if (!(result)) {
        fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Array definition must be an integer index.\n",token->lineNumber);
        return 0;
    }

    sizeAsString = token->lexeme; 
    sizeAsInteger = atoi(sizeAsString);
    setEntryArraySize(sizeAsInteger);
    return 1;
}


void parse_COMMANDS() {
    print_rule_to_file("COMMANDS -> COMMAND  COMMANDS_TAG");
    parse_COMMAND();
    parse_COMMANDS_TAG();
}


void parse_COMMANDS_TAG() {
    token = next_token();

    switch (token->kind) {
    case SEPARATOR_SEMECOLON:
        token = next_token();

        switch (token->kind) {
        case ID:
        case KEYWORD_WHEN:
        case KEYWORD_FOR: 
        case KEYWORD_FREE:
        case KEYWORD_BLOCK: 
            print_rule_to_file("COMMANDS_TAG -> ; COMMAND  COMMANDS_TAG");
            back_token();
            parse_COMMAND();
            parse_COMMANDS_TAG();
            break;

        case KEYWORD_END:
        case KEYWORD_DEFAULT:
        case KEYWORD_END_WHEN:
        case KEYWORD_END_FOR:
            print_rule_to_file("COMMANDS_TAG -> epsilon");
            back_token();
            back_token();
            break;

        default:
            errorHandler(follow_COMMANDS_TAG, token, "'ID', 'KEYWORD_WHEN', 'KEYWORD_FOR', 'KEYWORD_FREE', 'KEYWORD_BLOCK', 'KEYWORD_END', 'KEYWORD_DEFAULT', 'KEYWORD_END_WHEN', 'KEYWORD_END_FOR'", 0);
            break;
        }
        break;

    default:
        errorHandler(follow_COMMANDS_TAG, token, "'SEPARATOR_SEMICOLON'", 1);
        break;
    }
}


/* match_rel_op is used only in parse_COMMAND */
void match_rel_op() {
    token = next_token();

    switch (token->kind) {
    case REL_OP_LESS_OR_EQUAL:
    case REL_OP_GREATER_OR_EQUAL:
    case REL_OP_NOT_EQUAL:
    case REL_OP_GREATER:
    case REL_OP_LESS:
    case REL_OP_EQUAL:	
        break;
    default:
        errorHandler(follow_COMMAND, token, "'REL_OP_LESS_OR_EQUAL', 'REL_OP_GREATER_OR_EQUAL', 'REL_OP_NOT_EQUAL', 'REL_OP_GREATER', 'REL_OP_LESS', 'REL_OP_EQUAL'", 0);
        break;
    }
}


void parse_COMMAND() { 

    /* these variables are temp variables. they are used inside the parse_COMMAND for semantic checking */
    char* Id;
    char* typeName;
    char* entryType;
    char* tmpString;
    int receiverRootType, expressionRootType;	
    Entry receiverType, expressionType;
    Entry *entry, *idEntry, *typeNameEntry;
    int isValidTypeAndId;


    Token* tmpToken;
    token = next_token();

    switch (token->kind) {
    case KEYWORD_WHEN:
        print_rule_to_file("COMMAND -> when ( EXPRESSION rel_op EXPRESSION ) do COMMANDS ; default COMMANDS ; end_when"); 
        match(SEPARATOR_PARENTHESES_OPEN);

        /* first making sure that the type returned from parse_EXPRESSION is not undefined (that it has been found by the lookup) */

        if (!isValidType(parse_EXPRESSION().type)) {
			// error is already handled.
        }

        match_rel_op();

        if (!isValidType(parse_EXPRESSION().type)) { 
			// error is already handled.
        }

        match(SEPARATOR_PARENTHESES_CLOSE);
        match(KEYWORD_DO);
        parse_COMMANDS();
        match(SEPARATOR_SEMECOLON);
        match(KEYWORD_DEFAULT);
        parse_COMMANDS();
        match(SEPARATOR_SEMECOLON);
        match(KEYWORD_END_WHEN);
        break;

    case KEYWORD_FOR: 
        print_rule_to_file("COMMAND -> for ( id = EXPRESSION ; id rel_op EXPRESSION ;  id ++ ) COMMANDS ; end_for");
        match(SEPARATOR_PARENTHESES_OPEN);
        token = match(ID);
		if (!isIDdeclaredAndTypeNotExistInThisScope(token->lexeme)) {
			// error is already handled inside the method.
		} else {
			entry = lookupHirarchyAndReturnEntry(token->lexeme);
			if (entry) {
				if (entry->inheritedData) {
					if (entry->inheritedData->category == ARRAY_CATEGORY) {
						fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Assignment to array is forbidden. \n", token->lineNumber);
					} else if (entry->inheritedData->category == POINTER_CATEGORY) {
						fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Assignment to pointer is forbidden. \n", token->lineNumber);
                    }
                }
			}
		}
        match(ASSIGNMENT);

        expressionType = parse_EXPRESSION();

        /*  Making sure that the type returned from parse_EXPRESSION is not undefined. */
        if (isValidType(expressionType.type)) {
		   if (expressionType.category == ARRAY_CATEGORY || expressionType.category == POINTER_CATEGORY) { 
               fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: trying to assign an array or pointer is forbidden.\n", token->lineNumber); 
           }
        }
	
        /*   Checking that there is not array / pointer assignment.  */ 
       
        match(SEPARATOR_SEMECOLON);
        token = match(ID);
		if (isIDdeclaredAndTypeNotExistInThisScope(token->lexeme)) {
			// error is already handled inside the method.
		}
        match_rel_op();

        expressionType = parse_EXPRESSION(); 

        if (isValidType(expressionType.type)) {
		   if (expressionType.category == ARRAY_CATEGORY || expressionType.category == POINTER_CATEGORY) { 
               fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: trying to assign an array or pointer is forbidden.\n", token->lineNumber); 
           }
        }

        match(SEPARATOR_SEMECOLON);
        token = match(ID);
		if (isIDdeclaredAndTypeNotExistInThisScope(token->lexeme)) {
			// error is already handled inside the method.
		}
        match(OP_PLUS_PLUS);
        match(SEPARATOR_PARENTHESES_CLOSE);
        parse_COMMANDS();
        match(SEPARATOR_SEMECOLON);
        match(KEYWORD_END_FOR);
        break;

    case KEYWORD_FREE: 
        print_rule_to_file("COMMAND -> free (id)");
        match(SEPARATOR_PARENTHESES_OPEN);

        token = match(ID); 

        if (isIDdeclaredAndTypeNotExistInThisScope(token->lexeme)) { 
            // error handling is handled inside isIDdeclaredAndTypeNotExist... method
        }

        match(SEPARATOR_PARENTHESES_CLOSE);
        break;

    case KEYWORD_BLOCK:
        print_rule_to_file("COMMAND -> BLOCK");
        back_token();
        parse_BLOCK();
        break;

    case ID:

        Id = token->lexeme; 
  
        /*  Making sure the Id that is used in the left side of assignment is declared and that it is not a type name in this scope.  */
        if (!isIDdeclaredAndTypeNotExistInThisScope(token->lexeme)) {        
            isValidTypeAndId = 0; 
        } else {
            isValidTypeAndId = 1;
            idEntry = lookupHirarchyAndReturnEntry(token->lexeme); // idEntry is needed for later cases.
        }

        token = next_token();

        switch (token->kind) {
        case SEPARATOR_BRACKETS_OPEN: 
        case POINTER: 
            print_rule_to_file("COMMAND -> RECEIVER = EXPRESSION");

            back_token();
            back_token();

            receiverType = parse_RECEIVER();
            match(ASSIGNMENT);
            expressionType = parse_EXPRESSION();

            if (isValidTypeAndId) { 

                /*  CASE1: receiver is an array.  */
                if (receiverType.category == ARRAY_CATEGORY) {
                }

                /*  CASE2: expression is an array.  */
                if (expressionType.category == ARRAY_CATEGORY) {

                }

                /*  CASE3: default ERROR_TYPE.  */
                if (receiverType.type == DEFAULT_ERROR || expressionType.type == DEFAULT_ERROR) { 
                    fprintf(semanticFile_Ptr, "\n(Line %d). ERROR: Cannot perform assignment operation with an ERROR_TYPE entity.\n", token->lineNumber);
                }

                /*  CASE4: undefined ERROR_TYPE.  */
                if (expressionType.type == UNDEFINED_ID) { 
                   fprintf(semanticFile_Ptr, "\n(Line %d). ERROR: Cannot perform assignment operation with an ERROR_TYPE entity.\n", token->lineNumber);
                }

                if (NotValidReceiverAndExpression(receiverType, expressionType)) { 
                } else {
                    /*  CASE5:  pointer_to_var2 = &var2. so var1 must be a pointer to var2. */
                    if (expressionType.category == POINTER_CATEGORY && receiverType.category == POINTER_CATEGORY) { 
                        if ( (receiverAndExpressionPointersToTheSameType(receiverType, expressionType))) { 

                        } else { 
                            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment - different types.\n", token->lineNumber);
                        }

                        /*  CASE6:  pointer = some_basic_type. */
                    } else if (expressionType.category == NO_CATEGORY && receiverType.category == POINTER_CATEGORY) { 
                        fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: ,receiver a pointer and expression is of basic type.\n", token->lineNumber);
                    } else if (receiverType.category == NO_CATEGORY && expressionType.category == POINTER_CATEGORY) {
                        fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type basic, right side is of type pointer.\n", token->lineNumber); 

                       /*  CASE7:  var1 = var2. both var1 and var2 have some basic type in their root. a validation is done to make sure both of the same basic type. */
                    } else { 
                        /*  CASE1: expression is of basic type   */
                        if (expressionType.type == getTypeInt("integer") || expressionType.type == getTypeInt("real")) {
                            if (receiverType.type == expressionType.type || receiverType.subtype == expressionType.type) {
                            } else {
                                if (receiverType.type == expressionType.type) {
                                    
                                    fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type '%s', right side is of type '%s'.\n", 
										token->lineNumber, getTypeString(receiverType.type), getTypeString(expressionType.type));
                                } else {
                                    fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type '%s', right side is of type '%s'.\n", 
                                        token->lineNumber, getTypeString(receiverType.subtype), getTypeString(expressionType.type));
                                }
                            } 
                            /*  CASE2: receiver and expression both of user-defined-types with root basic types. make sure same types */
                        } else if (receiverType.type != expressionType.type) {
                            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type '%s', right side is of type '%s'.\n", 
                                token->lineNumber, getTypeString(receiverType.type), getTypeString(expressionType.type));
                        }							
                    }									
                }
            }

            break;

        case ASSIGNMENT: 
            token = next_token();

            switch (token->kind) {
            case KEYWORD_MALLOC:
                print_rule_to_file("COMMAND -> id = malloc ( size_of ( type_name ) ) ");
                match(SEPARATOR_PARENTHESES_OPEN);
                match(KEYWORD_SIZE_OF);
                match(SEPARATOR_PARENTHESES_OPEN); 

                typeName = match(ID)->lexeme;
				
                /*  Making sure the type_name exists.  */ 

                if (!isTypeExistInScopeHirarchy(typeName)) { 
                    fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' is not declared\n", token->lineNumber, typeName);
                }
                idEntry = lookupHirarchyAndReturnEntry(Id);

                if (idEntry) {
                    if (idEntry->inheritedData) {
                        if (idEntry->inheritedData->category == POINTER_CATEGORY && (idEntry->inheritedData->subtype == getTypeInt(typeName))) {
						} else if (idEntry->inheritedData->category == ARRAY_CATEGORY) {
							 fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Assignment to array is forbidden. '%s'\n", token->lineNumber, Id, typeName);
						} else {
							fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: id '%s' is not pointer to '%s'\n", token->lineNumber, Id, typeName);
                        }
                    }

                }


                match(SEPARATOR_PARENTHESES_CLOSE);
                match(SEPARATOR_PARENTHESES_CLOSE);
                break;

            case INT_NUM:
            case REAL_NUM:
            case ADDRESS_OF:
            case KEYWORD_SIZE_OF:
            case ID:
                print_rule_to_file("COMMAND -> RECEIVER = EXPRESSION");

                back_token();
                back_token(); 
                back_token();
                receiverType = parse_RECEIVER(); 
                match(ASSIGNMENT);
                expressionType = parse_EXPRESSION();

                if (isValidTypeAndId) {

                    /*  CASE1: receiver is an array.  */
                    if (receiverType.category == ARRAY_CATEGORY) {
                        fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Access to array must be by index.\n", token->lineNumber);
                    }

                    /*  CASE2: expression is an array.  */
                    if (expressionType.category == ARRAY_CATEGORY) {
                        fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Access to array must be by index.\n", token->lineNumber);
                        // consult: maybe give a better error msg ?
                    }

                    /*  CASE3: default ERROR_TYPE.  */
                    if (receiverType.type == DEFAULT_ERROR || expressionType.type == DEFAULT_ERROR) { 
                         fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type '%s', right side is of type '%s'.\n",
                             token->lineNumber, getTypeString(receiverType.type), getTypeString(expressionType.type));
                    }

                    /*  CASE4: undefined ERROR_TYPE.  */
                    if (expressionType.type == UNDEFINED_ID) { 
                        fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Cannot perform assignment operation on an ERROR_TYPE entity.'\n", token->lineNumber);
                    }

                    if (NotValidReceiverAndExpression(receiverType, expressionType)) {  //popopo
                    } else {

                        /*  CASE5:  pointer_to_var2 = &var2. so var1 must be a pointer to var2. var1 : pointer_to_var2, var2 : sometype, type pointer_to_var2 is ^sometype, type sometype is integer; */
                        if (expressionType.category == POINTER_CATEGORY && receiverType.category == POINTER_CATEGORY) { 
                            if ( (receiverAndExpressionPointersToTheSameType(receiverType, expressionType))) { 
                            } else { 
								 fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side and right side are pointers of different type. '%s'.\n",
                                    token->lineNumber, getTypeString(receiverType.type), getTypeString(expressionType.type));
                            }

                            /*  CASE6:  pointer = var   OR   var = pointer are ILLEGAL */
                        } else if ( (expressionType.category == NO_CATEGORY || expressionType.category == NO_CATEGORY ) && receiverType.category == POINTER_CATEGORY) { 
                            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type pointer, right side is of type basic.\n", token->lineNumber);
                        } else if (receiverType.category == NO_CATEGORY && expressionType.category == POINTER_CATEGORY) {
                            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type basic, right side is of type pointer.\n", token->lineNumber); 

                            /*  CASE7:  var1 = var2. both var1 and var2 have some basic type in their root. a validation is done to make sure both of the same basic type. */
                        } else { 
			
                            /*  CASE1: expression is of basic type   */
                            if (expressionType.type == getTypeInt("integer") || expressionType.type == getTypeInt("real")) {
                                if (receiverType.type == expressionType.type || receiverType.subtype == expressionType.type) {
                                } else {
                                    fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type '%s', right side is of type '%s'.\n",
                                        token->lineNumber, getTypeString(receiverType.type), getTypeString(expressionType.type));
                                }
                                /*  CASE2: receiver and expression both of user-defined-types with root basic types. make sure same types */
                            } else if (receiverType.type != expressionType.type) {
                                fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in assignment: left side is of type '%s', right side is of type '%s'.\n",
                                    token->lineNumber, getTypeString(receiverType.type), getTypeString(expressionType.type));
                            }							
                        }					
                    }		
                }
                break;

            default: 
                errorHandler(follow_COMMAND, token, "'KEYWORD_MALLOC', 'INT_NUM', 'REAL_NUM', 'ADDRESS_OF', 'KEYWORD_SIZE_OF', 'ID'", 0);
                break;
            }
            break;

        default:
            errorHandler(follow_COMMAND, token, "'SEPARATOR_BRACKETS_OPEN', 'ASSIGNMENT', 'POINTER'", 0);
            break;	
        }
        break;

    default:
        errorHandler(follow_COMMAND, token, "'KEYWORD_WHEN', 'KEYWORD_FOR', 'KEYWORD_FREE', 'KEYWORD_BLOCK', 'ID'", 0); 
        break;
    }
}


Entry parse_RECEIVER() { 
    char msg[250];
    char* oldLexeme;
    Entry receiverType, returnType, receiverTagType, *receiverTypePtr;

    print_rule_to_file("RECEIVER -> id RECEIVER_TAG");
    token = match(ID);

    oldLexeme = stringDuplication(token->lexeme);

    /* making sure this ID is not a name of a type */
    if (isTypeExistInThisScope(token->lexeme)) {
        returnType.type = DEFAULT_ERROR;
    }

    /*  Making sure this ID was declared. (is inside the symbol table). if it wasn't. an error_type is returned (using UNDEFINED_ID macro)  */
    if (lookupHirarchyAndReturnEntry(token->lexeme)) { 
        receiverType = *lookupHirarchyAndReturnEntry(token->lexeme);
    } else {
        receiverType.type = UNDEFINED_ID;
    }

    receiverTagType = parse_RECEIVER_TAG(receiverType);
    if (!isNotErrorType(receiverType)) { // error exist
        return receiverType;
    } else {
        returnType = receiverTagType; 
    }

    return returnType;
}


Entry parse_RECEIVER_TAG(Entry receiverType) {
    Entry returnType, expressionType;
    char *id = token->lexeme; 

    token = next_token();

    switch(token->kind){
    case SEPARATOR_BRACKETS_OPEN:
        print_rule_to_file("RECEIVER_TAG -> [EXPRESSION]");
        expressionType = parse_EXPRESSION();
		

		/*  CASE1: No Errors  */
		if (isNotErrorType(receiverType) && isNotErrorType(expressionType)) {
			if (receiverType.inheritedData) { 
				if  ((expressionType.type == getTypeInt("integer")) && receiverType.inheritedData->category == ARRAY_CATEGORY) { 
					returnType.subtype = receiverType.inheritedData->subtype;
					match(SEPARATOR_BRACKETS_CLOSE);
					return returnType;
				}
			} 
		}


		/*   CASE2: Errors exist   */
		if (!isNotErrorType(receiverType) || !isNotErrorType(expressionType)) {

		   fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Cannot use an ERROR_TYPE entity.\n", token->lineNumber, id);  
		}
        if (isNotErrorType(receiverType)) {
            if (!isNotErrorType(expressionType)) { // if at least one of the IDs in the expression is not defined, then the whole expression is not defined.
				// error is handled inside expression.
			} else if (expressionType.type != getTypeInt("integer")) {                
                fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Array '%s' index must be an integer.\n", token->lineNumber, id);  
			}
            if (receiverType.inheritedData) {
				if (receiverType.inheritedData->category != ARRAY_CATEGORY) {
					fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: when using ID[index], ID must be an array. %s is not an array.\n", token->lineNumber, getTypeString(receiverType.type));	
				}
			} else {
				fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: when using ID[index], ID must be an array. %s is not an array.\n", token->lineNumber, getTypeString(receiverType.type));	
			}
			returnType.type = DEFAULT_ERROR;
			match(SEPARATOR_BRACKETS_CLOSE);
            return returnType;
	     } else {
            match(SEPARATOR_BRACKETS_CLOSE);
			returnType.type = DEFAULT_ERROR;
            return returnType;
        }
        break;

    case POINTER:
        print_rule_to_file("RECEIVER_TAG -> ^");

		/* CASE1: no errors  */
		if (isNotErrorType(receiverType)) {
			if (receiverType.inheritedData) {
				if (receiverType.inheritedData->category == POINTER_CATEGORY) {
					returnType.type = receiverType.inheritedData->subtype;
					if (receiverType.inheritedData->inheritedData) {
						returnType.subtype = receiverType.inheritedData->inheritedData->subtype;
					}
					returnType.category = NO_CATEGORY;
					return returnType;
				}
			}
		}


		/* CASE2: Errors Exist  */ 
		if (isNotErrorType(receiverType)) {
			if (!receiverType.inheritedData) {
				fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: An attempt to use a '^' operator on non-pointer type.\n", token->lineNumber);		
				returnType.type = DEFAULT_ERROR; 
				return returnType;
			}
		} 

    	returnType.type = DEFAULT_ERROR;
		return returnType;
        break;

    case ASSIGNMENT:
        print_rule_to_file("RECEIVER_TAG -> epsilon"); 
        back_token();

        if (!isNotErrorType(receiverType)) {
            returnType = receiverType;
			return returnType;
        } else {
			if (!receiverType.inheritedData) { // for example: var1 : integer;
				returnType.type = receiverType.type;
				returnType.category = NO_CATEGORY;
				returnType.subtype = getTypeInt(""); // no subtype
			} else if (receiverType.inheritedData->category == BASIC_CATEGORY) { // for example: var1 : type1, type type1 is integer;
				returnType.subtype = receiverType.inheritedData->subtype;
				returnType.type = receiverType.type;
				returnType.category = NO_CATEGORY;
			} else if (receiverType.inheritedData->category == ARRAY_CATEGORY) { 
				fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Assignment to array '%s' is forbidden.\n", token->lineNumber, id);
				returnType.type = DEFAULT_ERROR;
			} else if (receiverType.inheritedData->category == POINTER_CATEGORY) {
				returnType.category = POINTER_CATEGORY; 
				returnType.subtype = receiverType.inheritedData->subtype;
				returnType.type = receiverType.type;
			} else {
				returnType = receiverType;
			}
		}
        return returnType;
        break;

    default: 
        errorHandler(follow_RECEIVER_TAG, token, "'SEPARATOR_BRACKETS_OPEN', 'POINTER', 'ASSIGNMENT'", 0);
        break;
    }
}


Entry parse_EXPRESSION() {
    Entry *tmpEntry;
    Entry returnType; 
    Entry expressionType;
    Entry expressionTagType;
    Token* tmpToken;

    token = next_token();

    switch (token->kind) {
    case INT_NUM:
        print_rule_to_file("EXPRESSION -> int_num");
        returnType.type = getTypeInt("integer");
        returnType.role = VARIABLE;
        returnType.size = 0;
        returnType.subtype = getTypeInt("");
        returnType.category = NO_CATEGORY;
        returnType.inheritedData = NULL;
        return returnType;
        break;

    case REAL_NUM:
        print_rule_to_file("EXPRESSION -> real_num");
        returnType.type = getTypeInt("real");
        returnType.role = VARIABLE;
        returnType.size = 0;
        returnType.subtype = getTypeInt("");
        returnType.category = NO_CATEGORY;
        returnType.inheritedData = NULL;
        return returnType;
        break;

    case ADDRESS_OF:
        print_rule_to_file("EXPRESSION -> & id");

        tmpToken = match(ID);

		if (!isIDdeclaredAndTypeNotExistInThisScope(tmpToken->lexeme)) { 
            returnType.type = UNDEFINED_ID;
            return returnType;
        }

        tmpEntry = lookupHirarchyAndReturnEntry(tmpToken->lexeme);
        returnType.category = POINTER_CATEGORY;
        returnType.type = (*tmpEntry).type;
        returnType.size = ADDRESS_INDICATOR;
        returnType.subtype = (*tmpEntry).type; 
		returnType.inheritedData = NULL;
        return returnType;
        break;

    case KEYWORD_SIZE_OF:
        print_rule_to_file("EXPRESSION -> size_of ( type_name )");
        match(SEPARATOR_PARENTHESES_OPEN);

        tmpToken = match(ID);

		if (!isTypeExistInScopeHirarchy(tmpToken->lexeme)) { 
			fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' is not declared\n", token->lineNumber, tmpToken->lexeme);
			returnType.type = DEFAULT_ERROR;
        } else {
            returnType.type = getTypeInt("integer");
        }
		returnType.inheritedData = NULL;
        match(SEPARATOR_PARENTHESES_CLOSE);
        return returnType;
        break; 
         
    case ID:
        print_rule_to_file("EXPRESSION -> id EXPRESSION_TAG"); //findme


        if (lookupHirarchyAndReturnEntry(token->lexeme)) {
            expressionType = *lookupHirarchyAndReturnEntry(token->lexeme); 
        } else {
			fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' is not defined.\n", token->lineNumber, token->lexeme);
            expressionType.type = UNDEFINED_ID;   
			if (isTypeExistInThisScope(token->lexeme)) {
				fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' can not be used in this context.\n", token->lineNumber, token->lexeme);
				expressionType.type = DEFAULT_ERROR;			
			}
        }

        if (isTypeExistInScopeHirarchy(token->lexeme)) { 
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' can not be used in this context.\n", token->lineNumber, token->lexeme);
            expressionType.type = DEFAULT_ERROR;
        }

        expressionTagType = parse_EXPRESSION_TAG(expressionType);
        returnType = expressionTagType;

        return returnType;
        break;

    default:
        errorHandler(follow_EXPRESSION, token, "'INT_NUM', 'REAL_NUM', 'ADDRESS_OF', 'KEYWORD_SIZE_OF', 'ID'", 0);
        returnType.type = DEFAULT_ERROR; 
        return returnType;
        break;
    }
}


Entry parse_EXPRESSION_TAG(Entry expressionType) {
    Entry returnType;
    Entry expressionType2, expressionType3;
    int isInheritedType;
    char *id = token->lexeme;

    token = next_token();

    switch(token->kind) {
    case SEPARATOR_BRACKETS_OPEN: 
        print_rule_to_file("EXPRESSION_TAG -> [ EXPRESSION ]"); 

        expressionType3 = parse_EXPRESSION();

		/*  CASE1: No Errors  */
		if (isNotErrorType(expressionType3) && isNotErrorType(expressionType)) {
			if (expressionType.inheritedData) { 		
				if  ((expressionType3.type == getTypeInt("integer")) && expressionType.inheritedData->category == ARRAY_CATEGORY) {  
					returnType.type = expressionType.inheritedData->subtype;
					match(SEPARATOR_BRACKETS_CLOSE);
					return returnType;
				}
			}
		}

		/*   CASE2: Errors exist  */
        if (isNotErrorType(expressionType)) {
			if (!isNotErrorType(expressionType3)) { // if at least one of the IDs in the expression is not defined, then the whole expression is not defined.
			} else if (expressionType3.type != getTypeInt("integer")) {                
                fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Array '%s' index must be an integer.\n", token->lineNumber, id);
			}
       
            if (expressionType.inheritedData) { // ID is of user-defined type.
				if (expressionType.inheritedData->category != ARRAY_CATEGORY) {
					fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: when using ID[index], ID must be an array. %s is not an array.\n", token->lineNumber, getTypeString(expressionType.type));	
				}
			} else {
				fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: when using ID[index], ID must be an array. %s is not an array.\n", token->lineNumber, getTypeString(expressionType.type));	
			}
			returnType.type = DEFAULT_ERROR;
			match(SEPARATOR_BRACKETS_CLOSE);
            return returnType;
        } else {
            match(SEPARATOR_BRACKETS_CLOSE);
			returnType.type = DEFAULT_ERROR;
            return returnType;
        }

        match(SEPARATOR_BRACKETS_CLOSE);
        return returnType;
		break;

    case POINTER:
        print_rule_to_file("EXPRESSION_TAG -> ^");

		/* CASE1: no errors */
		if (isNotErrorType(expressionType)) {
			if (expressionType.inheritedData) {
				if (expressionType.inheritedData->category == POINTER_CATEGORY) {
					returnType.type = expressionType.inheritedData->subtype;
					if (expressionType.inheritedData->inheritedData) {
						returnType.subtype = expressionType.inheritedData->inheritedData->subtype;
					}
					return returnType;
				} else {
					fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: An attempt to use a '^' operator on non-pointer type.\n", token->lineNumber);	
					returnType.type = DEFAULT_ERROR;
					return returnType;
				}
			}
		}

		/* CASE2: Errors Exist */
		if (isNotErrorType(expressionType)) {
			if (!expressionType.inheritedData) {
				fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: An attempt to use a '^' operator on non-pointer type.\n", token->lineNumber);		
				returnType.type = DEFAULT_ERROR; 
				return returnType;
			} 
		}
        returnType.type = DEFAULT_ERROR;
		return returnType;
        break;

    case AR_OP_ADD:
    case AR_OP_SUB:
    case AR_OP_POW:
    case AR_OP_MUL:
    case AR_OP_DIV:
        print_rule_to_file("EXPRESSION_TAG -> ar_op EXPRESSION");

        expressionType2 = parse_EXPRESSION();
		

		/* Error: If at least one of the expressions is of ERROR_TYPE. return error. */
		if (!isNotErrorType(expressionType2) || !isNotErrorType(expressionType)) {		
			fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: cannot ar_op on ERROR_TYPE.\n", token->lineNumber);
			returnType.type = DEFAULT_ERROR;
            return returnType;
        }

		
		/* Error: If at least one of the expressions is an array. return error */
        if (expressionType2.category == ARRAY_CATEGORY) {
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: cannot ar_op on array\n", token->lineNumber);
            returnType.type = DEFAULT_ERROR;
            return returnType;
        }
		if (expressionType.inheritedData) {
			if (expressionType.inheritedData->category == ARRAY_CATEGORY) { 
				fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: An arithmetic expression on array is not legal.\n", token->lineNumber);
				returnType.type = DEFAULT_ERROR;
				return returnType;
			}
		}

		/* Error: If at least one of the expressions is a pointer. return error */
		if (expressionType.inheritedData) { // for example if var1 + var2, and var1 is of user defined type
			if (expressionType.inheritedData->category == POINTER_CATEGORY) { // for example: ptr + something
				fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Arithmetic expressions can not be applied to arguments of pointer types.\n", token->lineNumber);
				returnType.type = DEFAULT_ERROR;
				return returnType;
			}
		}
        if ( expressionType2.category == POINTER_CATEGORY) {
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Arithmetic expressions can not be applied to arguments of pointer types.\n", token->lineNumber);
            returnType.type = DEFAULT_ERROR;
            return returnType;
        }
  

		if (expressionType.inheritedData) { 
			if (expressionType.inheritedData->category == BASIC_CATEGORY) { 

				/*  CASE1:  var1 ar_op var2,  var1 is of USER-DEFINED-TYPE, var2 is of basic type */
				if (isFromBasicType(expressionType2)) { 
					if (expressionType.inheritedData->subtype == expressionType2.type || expressionType.type == expressionType2.type) {
						returnType.type = expressionType2.type;								
						returnType.category = NO_CATEGORY;
						returnType.size = 0;
						returnType.subtype = getTypeInt("");
						return returnType;
					} else {
						handleArithmaticInconsistency(expressionType, expressionType2);
						returnType.type = DEFAULT_ERROR;
						return returnType;
					}
				

					/*  CASE2:  var1 ar_op var2,  var1 is of USER_DEFINED-TYPE, var2 is of USER_DEFINED_TYPE */
				} else {  
					if (expressionType.type == expressionType2.type) {
						returnType.type = expressionType.inheritedData->subtype;
						returnType.category = NO_CATEGORY;
						returnType.size = 0;
						returnType.subtype = getTypeInt("");
						return returnType;
					} else {							
						handleArithmaticInconsistency(expressionType, expressionType2);
						returnType.type = DEFAULT_ERROR;
						return returnType;
					}
				}
			} 

			/*  CASE3:  var1 ar_op var2,  var1 is of basic type and var2 is from basic type*/
		} else if (isFromBasicType(expressionType) && isFromBasicType(expressionType2)) {
			if (expressionType.type == expressionType2.type) {

				returnType.type = expressionType.type;
				returnType.category = NO_CATEGORY;
				returnType.size = 0;
				returnType.subtype = getTypeInt("");
				return returnType;
			} else {
				handleArithmaticInconsistency(expressionType, expressionType2);
				returnType.type = DEFAULT_ERROR;
				return returnType;
			}

			/*  CASE4:  var1 ar_op var2,  var1 is of basic type and var2 is of USER-DEFINED-TYPE*/
		} else if (isFromBasicType(expressionType) && !isFromBasicType(expressionType2)) {
			if (expressionType.type == expressionType2.type || expressionType.type == expressionType2.subtype) {
				returnType.type = expressionType.type;				
				returnType.category = NO_CATEGORY;
				returnType.size = 0;
				returnType.subtype = getTypeInt("");
				return returnType;
			} else {
				handleArithmaticInconsistency(expressionType, expressionType2);
				returnType.type = DEFAULT_ERROR;
				return returnType;
			}
		}
		returnType = expressionType;
		return returnType;
		break;

    case SEPARATOR_SEMECOLON: 
    case REL_OP_EQUAL:
    case REL_OP_NOT_EQUAL:
    case REL_OP_GREATER_OR_EQUAL:
    case REL_OP_LESS_OR_EQUAL:
    case REL_OP_GREATER:
    case REL_OP_LESS:
    case SEPARATOR_PARENTHESES_CLOSE:
    case SEPARATOR_BRACKETS_CLOSE:
        print_rule_to_file("EXPRESSION_TAG -> epsilon");
        back_token();
        returnType = expressionType; 


        /**  This is the case when expression contains only an ID. 
          *  let's find out if it is an array or a pointer (because if so, then: later ar_op
          *  on this ID will result in error). 
		  */
        if (!isNotErrorType(expressionType)) {
            returnType = expressionType;
        } else {
            /*  CASE1: expression is of some user-defined-type.   var1 : some-user-defined-type, */
            if (expressionType.inheritedData) {
                if (expressionType.inheritedData->category == ARRAY_CATEGORY) { 
					fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Assignment to array '%s' is forbidden.\n", token->lineNumber, id);
                    returnType.type = DEFAULT_ERROR;
                    returnType.category = ARRAY_CATEGORY;
                } else if (expressionType.inheritedData->category == POINTER_CATEGORY) { 
                    returnType.category = POINTER_CATEGORY; 
                    returnType.subtype = expressionType.inheritedData->subtype;
                    returnType.type = expressionType.type;
                } else if (expressionType.inheritedData->category == BASIC_CATEGORY) {
                    returnType.category = NO_CATEGORY;
                    returnType.type = expressionType.type;
                    returnType.subtype = returnType.inheritedData->subtype;
                }
            } else if (expressionType.inheritedData == NULL) { // for example: var1 : integer;
                returnType.type = expressionType.type;
                returnType.category = NO_CATEGORY;
                returnType.subtype = getTypeInt(""); 
            }
        }
        return returnType;
        break;

    default:   
        errorHandler(follow_EXPRESSION_TAG, token, "'SEPARATOR_BRACKETS_OPEN', 'POINTER', 'AR_OP_ADD', 'AR_OP_SUB', 'AR_OP_POW', 'AR_OP_MUL', 'AR_OP_DIV', 'SEPARATOR_SEMECOLON', 'REL_OP_EQUAL', 'REL_OP_NOT_EQUAL',  'REL_OP_GREATER_OR_EQUAL', 'REL_OP_LESS_OR_EQUAL',  'REL_OP_GREATER', 'REL_OP_LESS', 'SEPARATOR_PARENTHESES_CLOSE', 'SEPARATOR_BRACKETS_CLOSE'", 0 );
        returnType.type = DEFAULT_ERROR;
        return returnType;
        break;   
    }  
}


/**
* follow() Methods Implementations.
*/
int follow_PROGRAM(Token *token) {
    return (token->kind == END_OF_FILE);
}


int follow_BLOCK(Token *token) {
    return (token->kind == END_OF_FILE || token->kind == SEPARATOR_SEMECOLON);
}


int follow_DEFINITIONS(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_DEFINITIONS_TAG(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_DEFINITION(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_VAR_DEFINITION(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_VAR_DEFINITION_TAG(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_TYPE_DEFINITION(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_TYPE_INDICATOR(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_BASIC_TYPE(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_ARRAY_TYPE(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_POINTER_TYPE(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_POINTER_TYPE_TAG(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON); 
}


int follow_SIZE(Token *token) {
    return (token->kind == SEPARATOR_BRACKETS_CLOSE);
}


int follow_COMMANDS(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_COMMANDS_TAG(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_COMMAND(Token *token) {
    return (token->kind == SEPARATOR_SEMECOLON);
}


int follow_RECEIVER(Token *token) {
    return (token->kind == ASSIGNMENT);
}


int follow_RECEIVER_TAG(Token *token) {
    return (token->kind == ASSIGNMENT);
}


int follow_EXPRESSION(Token *token) {
    return (
        token->kind == SEPARATOR_SEMECOLON || 
        token->kind == REL_OP_EQUAL ||
        token->kind == REL_OP_NOT_EQUAL ||
        token->kind == REL_OP_GREATER_OR_EQUAL ||
        token->kind == REL_OP_LESS_OR_EQUAL ||
        token->kind == REL_OP_GREATER ||
        token->kind == REL_OP_LESS ||
        token->kind == SEPARATOR_PARENTHESES_CLOSE ||
        token->kind == SEPARATOR_BRACKETS_CLOSE
        );      
}                                    


int follow_EXPRESSION_TAG(Token *token) {
    return (
        token->kind == SEPARATOR_SEMECOLON || 
        token->kind == REL_OP_EQUAL ||
        token->kind == REL_OP_NOT_EQUAL ||
        token->kind == REL_OP_GREATER_OR_EQUAL ||
        token->kind == REL_OP_LESS_OR_EQUAL ||
        token->kind == REL_OP_GREATER ||
        token->kind == REL_OP_LESS ||
        token->kind == SEPARATOR_PARENTHESES_CLOSE ||
        token->kind == SEPARATOR_BRACKETS_CLOSE
        );       
}

/* helpers */

int saveEntryKey(char* key) { 
    Entry* lookupResult = lookupAndReturnEntry(key);
    if (isTypeIntegerOrReal(key)) {
        fprintf(semanticFile_Ptr, "\nError in line %d. Cannot use name '%s' as a type name. \n", token->lineNumber, key);
        return 0;
    }
    if (setEntryKey(key) == -1 || isTypeExistInThisScope(key)) { 
        if(lookupResult->role == VARIABLE){
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Variable '%s' already exists in current scope.\n", token->lineNumber, key);
        } else {
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' already exists in current scope.\n", token->lineNumber, key);
        }
        return 0;
    }
    return 1;
}

int isTypeExistInScopeHirarchy(char* lexeme) {
    Entry* lookupResult = lookupHirarchyAndReturnEntry(lexeme);

    if (lookupResult && (*lookupResult).role == USER_DEFINED_TYPE ) {
        return 1;
    } else {
        return 0;
    } 
}

int isTypeExistInThisScope(char* lexeme) {
    Entry* lookupResult = lookupAndReturnEntry(lexeme);
    if (!lookupResult) {
        return 0;
    } else if (lookupResult && (*lookupResult).role == USER_DEFINED_TYPE) {
        return 1;
    } else {
        return 0;
    }

}

int isValidType(int inputType) {
    if (inputType != UNDEFINED_ID && inputType != DEFAULT_ERROR) {
        return 1;
    } else {
        return 0;
    }
}

int isDeclaredID(char* Id) {
    Entry* idEntry = lookupHirarchyAndReturnEntry(Id); 

    if (!idEntry) {
        return 0;
    } else {
        return 1;
    }
}

int NotValidReceiverAndExpression(Entry receiverType, Entry expressionType) {
    if (
        receiverType.type == UNDEFINED_ID 
        || expressionType.type == UNDEFINED_ID 
        || receiverType.type == DEFAULT_ERROR 
        || expressionType.type == DEFAULT_ERROR
        || receiverType.category == ARRAY_CATEGORY
        || expressionType.category == ARRAY_CATEGORY
        ) {
            return 1; // not valid.
    } else {
        return 0; // valid.
    }
}

int receiverAndExpressionPointersToTheSameType(Entry receiver, Entry expression) { // in var1 = &var2 scenario. assumption: var2.type is set to its type (igonoring & inside parse_EXPRESSION)
    if (expression.size == ADDRESS_INDICATOR) {
        if (receiver.category == POINTER_CATEGORY && expression.category == POINTER_CATEGORY) {
            if (receiver.subtype == expression.subtype) {
                return 1;
            } else {
                return 0;
            }
        }
    } else {
        if (receiver.category == POINTER_CATEGORY && expression.category == POINTER_CATEGORY) {
            if (receiver.type == expression.type) {
                return 1;
            } else {
                return 0;
            }
        }
    }
}

/* This method does a validation on two things: 1. the ID has been declared.  2. there is no such type name in this scope.  */
int isIDdeclaredAndTypeNotExistInThisScope(char* name) {
    if (!isTypeExistInThisScope(name) && isDeclaredID(name)) {
        return 1;
    } else {
        if (!isDeclaredID(name)) {
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Variable '%s' is not defined.\n", token->lineNumber, name);
        } 
        if (isTypeExistInThisScope(name)) {
            fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Type '%s' can not be used in this context.\n", token->lineNumber, name);
        }
        return 0;
    }
}

int isTypeIntegerOrReal(char *type) {
    if ( strcmp(type, "integer") == 0 || strcmp(type, "real") == 0 ) {
        return 1;
    } else {
        return 0;
    } 
}

int isNotErrorType(Entry expression) {
	if (expression.type != UNDEFINED_ID && expression.type != DEFAULT_ERROR) {
		return 1;
	} else {
		return 0;
	}
}

void handleArithmaticInconsistency(Entry expression1, Entry expression2) {
	fprintf(semanticFile_Ptr, "\n(Line %d) ERROR: Inconsistency of types in arithmetic expression: left side is of type '%s', right side is of type '%s'.\n", 
	token->lineNumber,getTypeString(expression1.type),getTypeString(expression2.type));
}

int isFromBasicType(Entry expression) {
	if (expression.type == getTypeInt("integer") || expression.type == getTypeInt("real")) {
		return 1;
	} else {
		return 0;
	}
}



