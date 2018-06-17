#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "array.h"

/* an indicator for undefined id. */
#define UNDEFINED_ID -999

/* external vars */
extern TypeStringTable Global_typeStringTable;
extern FILE *semanticFile_Ptr, *HTMLsemanticFile_Ptr;



/******************
 * The Original API
 ******************/
struct map_node_t {
  unsigned hash;
  void *value;
  map_node_t *next;
};

static unsigned map_hash(const char *str) {
  unsigned hash = 5381;
  while (*str) {
    hash = ((hash << 5) + hash) ^ *str++;
  }
  return hash;
}


static map_node_t *map_newnode(const char *key, void *value, int vsize) {
  map_node_t *node;
  int ksize = strlen(key) + 1;
  int voffset = ksize + ((sizeof(void*) - ksize) % sizeof(void*));
  node = malloc(sizeof(*node) + voffset + vsize);
  if (!node) return NULL;
  memcpy(node + 1, key, ksize);
  node->hash = map_hash(key);
  node->value = ((char*) (node + 1)) + voffset;
  memcpy(node->value, value, vsize);
  return node;
}


static int map_bucketidx(map_base_t *m, unsigned hash) {
  /* If the implementation is changed to allow a non-power-of-2 bucket count,
   * the line below should be changed to use mod instead of AND */
  return hash & (m->nbuckets - 1);
}


static void map_addnode(map_base_t *m, map_node_t *node) {
  int n = map_bucketidx(m, node->hash);
  node->next = m->buckets[n];
  m->buckets[n] = node;
}


static int map_resize(map_base_t *m, int nbuckets) {
  map_node_t *nodes, *node, *next;
  map_node_t **buckets;
  int i; 
  /* Chain all nodes together */
  nodes = NULL;
  i = m->nbuckets;
  while (i--) {
    node = (m->buckets)[i];
    while (node) {
      next = node->next;
      node->next = nodes;
      nodes = node;
      node = next;
    }
  }
  /* Reset buckets */
  buckets = realloc(m->buckets, sizeof(*m->buckets) * nbuckets);
  if (buckets != NULL) {
    m->buckets = buckets;
    m->nbuckets = nbuckets;
  }
  if (m->buckets) {
    memset(m->buckets, 0, sizeof(*m->buckets) * m->nbuckets);
    /* Re-add nodes to buckets */
    node = nodes;
    while (node) {
      next = node->next;
      map_addnode(m, node);
      node = next;
    }
  }
  /* Return error code if realloc() failed */
  return (buckets == NULL) ? -1 : 0;
}


static map_node_t **map_getref(map_base_t *m, const char *key) {
  unsigned hash = map_hash(key);
  map_node_t **next;
  if (m->nbuckets > 0) {
    next = &m->buckets[map_bucketidx(m, hash)];
    while (*next) {
      if ((*next)->hash == hash && !strcmp((char*) (*next + 1), key)) {
        return next;
      }
      next = &(*next)->next;
    }
  }
  return NULL;
}


void map_deinit_(map_base_t *m) {
  map_node_t *next, *node;
  int i;
  i = m->nbuckets;
  while (i--) {
    node = m->buckets[i];
    while (node) {
      next = node->next;
      free(node);
      node = next;
    }
  }
  free(m->buckets);
}


void *map_get_(map_base_t *m, const char *key) {
  map_node_t **next = map_getref(m, key);
  return next ? (*next)->value : NULL;
}


int map_set_(map_base_t *m, const char *key, void *value, int vsize) {
  int n, err;
  map_node_t **next, *node;
  /* Find & replace existing node */
  next = map_getref(m, key);
  if (next) {
    memcpy((*next)->value, value, vsize);
    return 0;
  }
  /* Add new node */
  node = map_newnode(key, value, vsize);
  if (node == NULL) goto fail;
  if (m->nnodes >= m->nbuckets) {
    n = (m->nbuckets > 0) ? (m->nbuckets << 1) : 1;
    err = map_resize(m, n);
    if (err) goto fail;
  }
  map_addnode(m, node);
  m->nnodes++;
  return 0;
  fail:
  if (node) free(node);
  return -1;
}


void map_remove_(map_base_t *m, const char *key) {
  map_node_t *node;
  map_node_t **next = map_getref(m, key);
  if (next) {
    node = *next;
    *next = (*next)->next;
    free(node);
    m->nnodes--;
  }
}


map_iter_t map_iter_(void) {
  map_iter_t iter;
  iter.bucketidx = -1;
  iter.node = NULL;
  return iter;
}


const char *map_next_(map_base_t *m, map_iter_t *iter) {
  if (iter->node) {
    iter->node = iter->node->next;
    if (iter->node == NULL) goto nextBucket;
  } else {
    nextBucket:
    do {
      if (++iter->bucketidx >= m->nbuckets) {
        return NULL;
      }
      iter->node = m->buckets[iter->bucketidx];
    } while (iter->node == NULL);
  }
  return (char*) (iter->node + 1);
}





/************************************
 * The Customization to Symbol Tables
 ************************************/


void PrintSymbolTableElement(Entry value, char* key) {
	char subtypeString[50];
	char typeString[50];
	char *type;

	sprintf(subtypeString, "%d", value.subtype);
	sprintf(typeString, "%d", value.type);
	if (!typeString) {
		return;
	}
		type = *map_get(&(Global_typeStringTable.typeStrTable), typeString);
		/*printf(
        "%17s %17s %17s %17s %17s %17.0d\n",
        key,
        roleToString(value.role),
		type,
        categoryToString(value.category),
		*map_get(&(Global_typeStringTable.typeStrTable), subtypeString), 
        value.size 
        );*/

	fprintf(HTMLsemanticFile_Ptr,
        "<tr>\
	        <th>%s</th>\
	        <td>%s</td>\
	        <td>%s</td>\
	        <td>%s</td>\
	        <td>%s</td>\
	        <td>%15.0d</td>\
        </tr>",
        key,
        roleToString(value.role),
		type,
        categoryToString(value.category),
		*map_get(&(Global_typeStringTable.typeStrTable), subtypeString), 
        value.size 
        );
}


void printTable(SymbolTable symbolTable) {
    const char *keyI;
    map_iter_t iter;
    Entry myValue;
    int i;
    
    iter = map_iter(&symbolTable);
	
    //for (i = 0;  i < 105;  i++, printf("%c", '-'));
	//printf("\n%15s   %15s   %15s   %15s   %15s   %15s\n", "ID","Role", "Type", "Category", "Subtype", "Size");
    //for (i = 0;  i < 105;  i++, printf("%c", '='));
    //printf("\n");
    while ((keyI = map_next(&symbolTable, &iter))) {
	   myValue = *map_get(&symbolTable, keyI);
       PrintSymbolTableElement( myValue, keyI);
    }
    //for (i = 0;  i < 105;  i++, printf("%c", '-'));
    //printf("\n\n");

	
}

char *roleToString(enum Role role) {
    char *strings[] = { "VARIABLE", "USER_DEFINED_TYPE" };
    return strings[role];
}

char *categoryToString(enum Category category) {
	char *strings[] = { "", "basic", "array", "pointer" };
    return strings[category];
}



/***********************
 Extra Type String Table
************************/

/* Add new type to type symbol. */
void addToTypeTable(TypeStringTable* typeStringTable, char *type) {
    char counterString[50]; // holds the string of int
	static int counter = 0;

	if(!m_isset((*typeStringTable).typeIntTable, type)) { // key does not exist -> create it
		m_set((*typeStringTable).typeIntTable, type, (*typeStringTable).counter);
        sprintf(counterString, "%d", (*typeStringTable).counter); 
		m_set((*typeStringTable).typeStrTable, counterString, type);
        (*typeStringTable).counter++;  
    }
}

void printTypeStrTable(map_str_t m) {
    int i;
    const char *key;
    map_iter_t iter = map_iter(&m);

    //printf("-----------------------------------------------------------------------------------------------\n");
    //printf("                                   Type Table: Integer -> String \n");

    //for (i = 0;  i < 95;  i++, printf("%c", '-'));
    //printf("\n%15s %15s\n", "Key","Value");
    //for (i = 0;  i < 95;  i++, printf("%c", '='));
    //printf("\n");

    // START of html table
    fprintf(HTMLsemanticFile_Ptr, 
        "<h3 style=\'text-align: center;\'>Type Table: integer -> string</h3>\
        <table class=\'table table-striped\' style=\'border: 2px solid black;\'>\
            <thead class=\'thead-dark\'>\
                <tr>\
                    <th scope=\'col\'>Key</th>\
                    <th scope=\'col\'>Val</th>\
                </tr>\
            </thead>\
            <tbody>"
    );

    while ((key = map_next(&m, &iter))) {
        //printf( "%15s %15s\n", key, *map_get(&m, key) );

        fprintf(HTMLsemanticFile_Ptr,
        "<tr>\
	        <th>%s</th>\
	        <td>%s</td>\
        </tr>",
        key,
        *map_get(&m, key) 
        );
    }


    // END of html table
    fprintf(HTMLsemanticFile_Ptr, "</tbody></table>"); 

    //for (i = 0;  i < 95;  i++, printf("%c", '-'));
    //printf("\n\n");
}

void printTypeIntTable(map_int_t m) {
    int i;
    const char *key;
    map_iter_t iter = map_iter(&m);

    //printf("-----------------------------------------------------------------------------------------------\n");
    //printf("                                   Type Table: String -> Int \n");

    //for (i = 0;  i < 95;  i++, printf("%c", '-'));
    //printf("\n%15s %15s\n", "Key","Value");
    //for (i = 0;  i < 95;  i++, printf("%c", '='));
    //printf("\n");

    // START of html table
    fprintf(HTMLsemanticFile_Ptr, 
        "<h3 style=\'text-align: center;\'>Type Table: string -> integer</h3>\
        <table class=\'table table-striped\' style=\'border: 2px solid black;\'>\
            <thead class=\'thead-dark\'>\
                <tr>\
                    <th scope=\'col\'>Key</th>\
                    <th scope=\'col\'>Val</th>\
                </tr>\
            </thead>\
            <tbody>"
    );

    while ((key = map_next(&m, &iter))) {
        //printf( "%15s %15d\n", key, *map_get(&m, key) );

        fprintf(HTMLsemanticFile_Ptr,
        "<tr>\
	        <th>%s</th>\
	        <td>%d</td>\
        </tr>",
        key,
        *map_get(&m, key) 
        );
    }

    // END of html table
    fprintf(HTMLsemanticFile_Ptr, "</tbody></table>"); 

    //for (i = 0;  i < 95;  i++, printf("%c", '-'));
    //printf("\n\n");
}





/* Init type table. */

void initTypeStrTable() {
	map_init(&Global_typeStringTable.typeIntTable);
	map_init(&Global_typeStringTable.typeStrTable);
	addToTypeTable(&Global_typeStringTable, "integer");
    addToTypeTable(&Global_typeStringTable, "real");
	addToTypeTable(&Global_typeStringTable, "");
}
 
/* retrieving the int representation of the type */
int getTypeInt(char* theType) { 
	if (map_get(&(Global_typeStringTable.typeIntTable), theType)) {
		return  m_get(Global_typeStringTable.typeIntTable, theType);
	} else {
		return UNDEFINED_ID; 
	}
}

char* getTypeString(int typeAsInt) {
	char typeString[50];
	char *typeAsString;
		
	sprintf(typeString, "%d", typeAsInt);
    if(typeAsInt == -998){
        return "error_type";
    }
	if (map_get(&(Global_typeStringTable.typeStrTable), typeString)) {
		return *map_get(&(Global_typeStringTable.typeStrTable), typeString); 
	} else {
		return NULL;
	}	
}

void cleanTypeTable() {
	map_deinit(&Global_typeStringTable.typeStrTable);
	map_deinit(&Global_typeStringTable.typeIntTable);
	Global_typeStringTable.counter = 0;
}