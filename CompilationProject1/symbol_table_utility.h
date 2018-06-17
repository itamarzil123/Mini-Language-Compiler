/*
|-------------------------------------------------------...------------------------------------------
| Symbol Table Utility Module Header. This module contains helpers methods for managing symbol tables.
|--------------------------------------------------------...-----------------------------------------
|
*/
#pragma once

#ifndef SYMBOL_TABLE_UTILITY_H
#define SYMBOL_TABLE_UTILITY_H

#define curSymbolTable(scopeArr) scopeArr.array[scopeArr.curScopeIndex].symbolTable

#include "array.h"
#include "map.h"

/* lookups */
int lookup(char* key);
Entry* lookupAndReturnEntry(char* key);

/* lookups in the scope hirarchy */
int lookupHirarchy(char* key);
Entry* lookupHirarchyAndReturnEntry(char* key);

Entry *newEntry(enum Role role, int type, enum Category category, char *subtype, int size);

/* for duplicating string.*/
char *stringDuplication (const char *s);

/* setters */
void setEntryInheritedData(Entry* lookupResult);
int setEntryKey(char* key);
void setEntryRole(enum Role role);
void setEntryType(char* type);
void setEntryCategory(enum Category category);
void setEntrySubtype(char* subtype);
void setEntryArraySize(int size);
/* getters */

void exitGlobalScope();


/* others */
void updateGlobalTempSymbolTable();
void insertSymbolTableToScopeArray(int parent);
void newScope();
void exitScope();

 
/************************
 * STACK implementation.*
 ************************/

int isEmpty(struct StackNode *root);
void push(struct StackNode** root, int data);
int pop(struct StackNode** root);
int peek(struct StackNode* root);
void initScopeStack();

#endif