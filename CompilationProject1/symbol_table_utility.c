#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "map.h"
#include "array.h"

#define UNDEFINED_ID -999
/* external vars needed */
extern ScopeArray Global_scopeArrayOfSymbolTables;
extern SymbolTable Global_tempSymbolTable; // tmp symbol table
extern Entry Global_tempEntry; // tmp symbol table value
extern char* Global_tempSymbolTableKey; // tmp symbol table key
extern ScopeElement Global_newScopeElement;
extern ScopeElement* Global_newScopeElementPtr; // TEMP VAR: each time a new symbol table is created, Global_newScopeElement will also be used.
extern SymbolTable* Global_currentSymbolTablePtr;
extern struct StackNode* Global_currentScopeStack;
extern TypeStringTable Global_typeStringTable;
extern FILE *semanticFile_Ptr, *HTMLsemanticFile_Ptr;


/* exiting global scope, printing output and freeing memory */
void exitGlobalScope(){
    //printf("Finished Parsing. Symbol Tables:\n");
	printScopeArray(&Global_scopeArrayOfSymbolTables);

    fprintf(HTMLsemanticFile_Ptr, "<br><br><br><hr><br><br><br>"); 
	
    printTypeStrTable(Global_typeStringTable.typeStrTable);
	printTypeIntTable(Global_typeStringTable.typeIntTable);

    fprintf(HTMLsemanticFile_Ptr, "</div></body></html>");

	freeScopeArray(&Global_scopeArrayOfSymbolTables);
}

Entry *newEntry(enum Role role, int type, enum Category category, char *subtype, int size) {
	Entry entry = { role, type, category, subtype, size };
    return &entry;
}

void updateGlobalTempSymbolTable() {
	m_set(*Global_currentSymbolTablePtr, Global_tempSymbolTableKey, Global_tempEntry);
}


/**
 * Setters 
 */

void setEntryInheritedData(Entry* lookupResult) {
	if (lookupResult != NULL) {
		Global_tempEntry.inheritedData = (Entry*) malloc(sizeof(Entry));
	}
	Global_tempEntry.inheritedData = lookupResult;

}

int setEntryKey(char* key) {
	int currentScope = peek(Global_currentScopeStack);
	if (lookup(key) == NULL) {
		Global_tempSymbolTableKey = strdup(key);
		return 1;
	} else {
		return -1;
	}
}

void setEntryRole(enum Role role) {
	Global_tempEntry.role = role;
}

void setEntryType(char* type) {
	if (m_isset(Global_typeStringTable.typeIntTable, type)) {
		Global_tempEntry.type = getTypeInt(type);
	}
}



void setEntryCategory(enum Category category) {
	Global_tempEntry.category = category;
}

void setEntrySubtype(char* subtype) {
	if (m_isset(Global_typeStringTable.typeIntTable, subtype)) {
      Global_tempEntry.subtype = getTypeInt(subtype);
	}
}

void setEntryArraySize(int size) {
	Global_tempEntry.size = size;
}

/* methods for finding keys in scopes */
int lookup(char* key) {
	Entry* returnEntry;
	int currentScope = peek(Global_currentScopeStack);

	returnEntry = map_get(&Global_scopeArrayOfSymbolTables.array[currentScope].symbolTable, key);

	if(returnEntry) {
		return 1;
	} else {
		return 0;
	}
}

Entry* lookupAndReturnEntry(char* key) {
	Entry* returnEntry;
	int currentScope = peek(Global_currentScopeStack);

	returnEntry = map_get(&Global_scopeArrayOfSymbolTables.array[currentScope].symbolTable, key);

	if(returnEntry) {
		return returnEntry;
	} else {
		return NULL;
	}
}
int lookupHirarchy(char* key) { 
	Entry* returnEntry;
	int currentScope = peek(Global_currentScopeStack);

	while (currentScope != -1) {
		returnEntry = map_get(&Global_scopeArrayOfSymbolTables.array[currentScope].symbolTable, key);
		if (returnEntry) {
			return 1;
		} 
		currentScope = Global_scopeArrayOfSymbolTables.array[currentScope].parent;
	}
	return 0;
}

Entry* lookupHirarchyAndReturnEntry(char* key) {
	Entry* returnEntry;

	int currentScope = peek(Global_currentScopeStack);

	while (currentScope != -1) {
		returnEntry = map_get(&Global_scopeArrayOfSymbolTables.array[currentScope].symbolTable, key);
		if(returnEntry) {
			return returnEntry; //boom
		} else {
			currentScope = Global_scopeArrayOfSymbolTables.array[currentScope].parent;
		}
	}
	return NULL;			
} 

/* getters */



/* implementation of string duplication */
char *stringDuplication (const char *s) {
    char *d = (char*) malloc (strlen (s) + 1);   // Space for length plus nul
    if (d == NULL) return NULL;          // No memory
    strcpy (d,s);                        // Copy the characters
    return d;                            // Return the new string
}






/***********************************************************
 * STACK implementation. (helps us tracking current scope) *
 ***********************************************************/ 

struct StackNode {
    int data;
    struct StackNode* next;
};
 
struct StackNode* newNode(int data) {
    struct StackNode* stackNode =
              (struct StackNode*) malloc(sizeof(struct StackNode));
    stackNode->data = data;
    stackNode->next = NULL;
    return stackNode;
}
 
int isEmpty(struct StackNode *root) {
    return !root;
}
 
void push(struct StackNode** root, int data) {
    struct StackNode* stackNode = newNode(data);
    stackNode->next = *root;
    *root = stackNode;
}
 
int pop(struct StackNode** root) {
	int popped;
	struct StackNode* temp;

    if (isEmpty(*root))
        return INT_MIN;
    temp = *root;
    *root = (*root)->next;
    popped = temp->data;
    free(temp);
 
    return popped;
}
 
int peek(struct StackNode* root) {
    if (isEmpty(root))
        return INT_MIN;
    return root->data;
}

void initScopeStack() {
	push(&Global_currentScopeStack, -1);
}



/* more scope managing methods */

void newScope() {
	int currentScope, parentScope;
	/* creating new scope element, init its symbol table */
	Global_newScopeElementPtr = (ScopeElement*) malloc(sizeof(ScopeElement));
	map_init(&(Global_newScopeElementPtr->symbolTable));
	parentScope = peek(Global_currentScopeStack);
	Global_newScopeElementPtr->parent = parentScope;

	/* inserting new and empty scope element to array of scopes.*/
	insertScopeArray(&Global_scopeArrayOfSymbolTables, *Global_newScopeElementPtr);
	// newScope() method should contain all above.
	push(&Global_currentScopeStack, Global_scopeArrayOfSymbolTables.used - 1);

	currentScope = peek(Global_currentScopeStack);

	Global_currentSymbolTablePtr = & ( Global_scopeArrayOfSymbolTables.array[currentScope].symbolTable );

	/* if we are in the global scope. let's add integer, real as types */
	if (currentScope == 0) {
		// setEntryKey("integer");
		// setEntryRole(USER_DEFINED_TYPE);
		// setEntryCategory(BASIC_CATEGORY);
		// setEntryArraySize(0);	
		// updateGlobalTempSymbolTable();
		// m_set(*Global_currentSymbolTablePtr, Global_tempSymbolTableKey, Global_tempEntry);
	}
	//printf("entering scope: %d", currentScope); 
}


void exitScope() {
	int currentScope = pop(&Global_currentScopeStack);

	//printf("\nExiting Scope: %d", currentScope);

	if (! currentScope) { // quiting global scope.
		exitGlobalScope();
	}
}