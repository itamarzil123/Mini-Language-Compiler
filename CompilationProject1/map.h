/*
|-------------------------------------------------------...----------------------------------------------------------------
| Map Module Header. This module contains an original hash map implementation in c and also the adaptation to symbol tables . 
|--------------------------------------------------------...---------------------------------------------------------------
|
*/

#pragma once

#ifndef MAP_H
#define MAP_H

#define m_set(m, key, val) map_set(&m, key, val)
#define m_get(m, key) *map_get(&m, key)
#define m_isset(m, key) map_get(&m, key)

#include <string.h>




/**
  * **************************************
  * Original API: Copyright (c) 2014 rxi
  * **************************************
  */

#define MAP_VERSION "0.1.0"

struct map_node_t;
typedef struct map_node_t map_node_t;

typedef struct {
    map_node_t **buckets;
    unsigned nbuckets, nnodes;
} map_base_t;

typedef struct {
    unsigned bucketidx;
    map_node_t *node;
} map_iter_t;

#define map_t(T)\
struct { map_base_t base; T *ref; T tmp; }

#define map_init(m)\
    memset(m, 0, sizeof(*(m)))

#define map_deinit(m)\
    map_deinit_(&(m)->base)

#define map_get(m, key)\
    ( (m)->ref = map_get_(&(m)->base, key) )

#define map_set(m, key, value)\
    ( (m)->tmp = (value),\
    map_set_(&(m)->base, key, &(m)->tmp, sizeof((m)->tmp)) )

#define map_remove(m, key)\
    map_remove_(&(m)->base, key)

#define map_iter(m)\
    map_iter_()

#define map_next(m, iter)\
    map_next_(&(m)->base, iter)

void map_deinit_(map_base_t *m);
void *map_get_(map_base_t *m, const char *key);
int map_set_(map_base_t *m, const char *key, void *value, int vsize);
void map_remove_(map_base_t *m, const char *key);
map_iter_t map_iter_(void);
const char *map_next_(map_base_t *m, map_iter_t *iter);

typedef map_t(void*) map_void_t;
typedef map_t(char*) map_str_t;
typedef map_t(int) map_int_t;
typedef map_t(char) map_char_t;
typedef map_t(float) map_float_t;
typedef map_t(double) map_double_t;





/**
  * *******************************************
  * New API: The Customization to Symbol Tables
  * *******************************************
  */

typedef enum Role {
    VARIABLE, 
	USER_DEFINED_TYPE
} Role;

/* Catagory Of user-defined types can be basic, array, or pointer;*/
typedef enum Category { // Category of user-defined types only !
    NO_CATEGORY, 
	BASIC_CATEGORY, 
	ARRAY_CATEGORY, 
	POINTER_CATEGORY  // NoCategory for NON user defined types.
} Category;


typedef struct Entry{ 
    Role role; // variable or user-defined type
	Category category; // only for user-defined types (basicCatagory, arrayCatagory, pointerCatagory)
    int type; // integer, real, or user-defined-type
    int subtype;  
    int size; // This attribute is relevant only for a user-defined type whose category is array.
	struct Entry* inheritedData; // a pointer to the 'inherited', and not synthesized data.
} Entry;

typedef map_t(Entry) SymbolTable;


/* Conversion between enum to string */
char *roleToString(enum Role role);

/* Conversion between enum to string */
char *categoryToString(enum Category Category);

/*
* Printing an entire symbol table
*/
void printTable(SymbolTable symbolTable);

/*
* Printing a single symbol table element (used inside printTable() )
*/
void PrintSymbolTableElement(Entry value, char* key);




/*************************************************************
 Extra Type String Table for managing types with O(1) ability
**************************************************************/
typedef struct  {
	map_str_t typeStrTable;
	map_int_t typeIntTable;
	int counter;
} TypeStringTable;

/* init type string table. */
void initTypeStrTable(); 

void cleanTypeTable();

/* insert */
void addToTypeTable(TypeStringTable* typeStringTable, char *type);

/* printers */
void printTypeStrTable(map_str_t m);
void printTypeIntTable(map_int_t m);

/* geters */
int getTypeInt(char* theType);
char* getTypeString(int typeAsInt);

#endif