/* Copyright (C) 2018 Doron Mor & Itamar Silverstein - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the HIT license.
 */
/*
|-------------------------------------------------------...---------------------------------------------------------------------
| Array Module Header. This module contains the dynamic array original API and also the adaptation to array of scopes. (see bottom).
|--------------------------------------------------------...--------------------------------------------------------------------
|
*/

#pragma once

#ifndef ARRAY_H
#define ARRAY_H

#include "map.h"



/**
  **************************
  * Original API (int Array)
  **************************
  */

typedef struct {
    int *array;
    size_t used; // number of elements in use
    size_t size; // actual array size 
} Array;

void initArray(Array *arr, size_t initialSize);
void insertArray(Array *arr, int element);
void freeArray(Array *arr);







/**
  * ************************
  * New API ( Scope Array )
  * ************************
  */

typedef struct {
    SymbolTable symbolTable; // hash table representing symbol table
    int parent; // parent of this scope
} ScopeElement;

typedef struct {
    ScopeElement *array; // array of scopes
    size_t used; // number of elements in use
    size_t size; // actual array size 
} ScopeArray;



void initScopeArray(ScopeArray *scopeArr, size_t initialSize);
void insertScopeArray(ScopeArray *scopeArr, ScopeElement scopeElement);
void freeScopeArray(ScopeArray *scopeArr);
void printScopeArray(ScopeArray *scopeArr);

#endif