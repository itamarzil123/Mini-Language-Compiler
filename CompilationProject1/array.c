#include <stdio.h>
#include <stdlib.h>
#include "array.h"
#include "map.h"

// datetime: 19.05.2018 15:45

/**
  * ************************
  * Original API (int Array)
  * ************************
  */

extern FILE *HTMLsemanticFile_Ptr, *semanticFile_Ptr;

void initArray(Array *a, size_t initialSize) {
    a->array = (int *)malloc(initialSize * sizeof(int));
    a->used = 0;
    a->size = initialSize;
}

/**
  * Insert array
  * 
  * a->used is the number of used entries, 
  * because a->array[a->used++] updates a->used only *after* the array has been accessed.
  * Therefore a->used can go up to a->size.
  */
void insertArray(Array *a, int element) {
    if (a->used == a->size) { // need to resize
        a->size *= 2;
        a->array = (int *)realloc(a->array, a->size * sizeof(int));
    }
    a->array[a->used++] = element;
}

void freeArray(Array *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}



/**
  * ****************************************
  * New API ( Customization to Scope Array )
  * ****************************************
  */

void initScopeArray(ScopeArray *a, size_t initialSize) {
    a->array = (ScopeElement *)malloc(initialSize * sizeof(ScopeElement));
    a->used = 0;
    a->size = initialSize;

}

/* Inserting new scopeElement to the scope array */
void insertScopeArray(ScopeArray *a, ScopeElement element) {
    if (a->used == a->size) { // need to resize
        a->size *= 2;
        a->array = (ScopeElement *)realloc(a->array, a->size * sizeof(ScopeElement));
    }
    a->array[a->used++] = element;
}

/* freeing memory */
void freeScopeArray(ScopeArray *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

/**
  * Print the scope array to console.
  * first table is *always* the global scope and it has no parent (-1). 
  */
void printScopeArray(ScopeArray *scopeArr) {
    size_t i;
    int k;

    for (i = 0; i < scopeArr->used; i++) {
        //for (k = 0;  k < 107;  k++, printf("%c", '-')); printf("\n");
        //printf("                               Symbol Table Number: %d Parent: %d \n", i, scopeArr->array[i].parent);
		
        //for (k = 0;  k < 107;  k++, printf("%c", '-')); printf("\n");

        // START of html table
        fprintf(HTMLsemanticFile_Ptr, 
            "<h3 style=\'text-align: center;\'>Table ID: %d, Parent ID: %d </h3>\
            <table class=\'table table-striped\' style=\'border: 2px solid black;\'>\
                <thead class=\'thead-dark\'>\
                    <tr>\
                        <th scope=\'col\'>ID</th>\
                        <th scope=\'col\'>Role</th>\
                        <th scope=\'col\'>Type</th>\
                        <th scope=\'col\'>Categoty</th>\
                        <th scope=\'col\'>Subtype</th>\
                        <th scope=\'col\'>Size</th>\
                    </tr>\
                </thead>\
                <tbody>",
            i,
            scopeArr->array[i].parent
        );
		
        printTable(scopeArr->array[i].symbolTable);
	
        // END of html table
        fprintf(HTMLsemanticFile_Ptr, "</tbody></table>"); 
    }
}