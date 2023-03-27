#include <pthread.h>

extern pthread_mutex_t *locks; 

// A hashmap entry stores the key, value
// and a pointer to the next entry
typedef struct ts_entry_t {
   int key;
   int value;
   struct ts_entry_t *next;
} ts_entry_t;

// A hashmap contains an array of pointers to entries,
// the capacity of the array, and the size (number of entries stored)
typedef struct ts_hashmap_t {
   ts_entry_t **table;
   int capacity;
   int size;
} ts_hashmap_t;

// function declarations
ts_hashmap_t *initmap(int capacity);
int get(ts_hashmap_t *map, int key);
int put(ts_hashmap_t *map, int key, int value);
int del(ts_hashmap_t *map, int key);
double lf(ts_hashmap_t *map);
void printmap(ts_hashmap_t *map);