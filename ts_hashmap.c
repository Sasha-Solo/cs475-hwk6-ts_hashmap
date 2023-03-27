#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_hashmap.h"

/**
* Globals
**/
pthread_mutex_t *locks; //array of locks

/**
 * Creates a new thread-safe hashmap. 
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity) {
  
   //malloc space  
   ts_hashmap_t *initial = (ts_hashmap_t*) malloc (1 * sizeof(ts_hashmap_t));

   //initialize args inside the struct
   initial->capacity = capacity;  //initialize capacity 
   initial->size = 0; //set size to 0 because no entry is stored yet

   //malloc space for table
   initial->table = (ts_entry_t**) malloc(capacity * sizeof(ts_entry_t*));
  
   for (int i = 0; i < capacity; i++) { //set all slots to null initially
     initial->table[i] = NULL; 
   }

   //get space for array to hold locks
	 locks = (pthread_mutex_t*) malloc (sizeof(pthread_mutex_t) * capacity);

	 for (int i = 0; i < capacity; i++){ //initialize mutexes inside locks array
		 pthread_mutex_init(&locks[i], NULL);
	 }
  
   return initial; 
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key) {
   // TODO
   unsigned int slot = (unsigned int) key; //cast key as unsigned int
   int index = slot % (map->capacity); //get index 

   pthread_mutex_lock(&locks[index]); //lock row with a lock at given index

   ts_entry_t *entry = map->table[index]; //get given entry

   if (entry == NULL){ //if entry is null, return INT_MAX
     pthread_mutex_unlock(&locks[index]); //unlock row
     return INT_MAX;
   }
   else{
    ts_entry_t *prev;
    while (entry != NULL){ //if entry exists...
      prev = entry;
      if (key == entry->key){ //check if given key matches entry key
        pthread_mutex_unlock(&locks[index]); //unlock row
        return entry->value;
      }
      entry = prev->next;
    }
   }
   pthread_mutex_unlock(&locks[index]); //unlock row
   return INT_MAX;
}

/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value) {
  //TODO
  unsigned int slot = (unsigned int) key; //cast key as unsigned int
  int index = slot % (map->capacity); //get index 

  pthread_mutex_lock(&locks[index]); //lock row with a lock at given index

  ts_entry_t *entry = map->table[index]; //get given entry
  
  if (entry == NULL){ //if entry is empty...
    //printf("\nentry is null\n");
     //get space for ts_entry_t struct
     entry = (ts_entry_t*) malloc (1 * sizeof(ts_entry_t)); 
     entry->key = key;  //initialize key with what was passed in 
     entry->value = value; //initialize value with what was passed in 
     entry->next = NULL; //set next pointer to null
     map->table[index] = entry; //insert the entry
     map->size++; //increment size

     pthread_mutex_unlock(&locks[index]); //unlock row
     return INT_MAX;
   }

   ts_entry_t *prev; //will be used to move along entries during while loop
   int oldValue = 0;
   while (entry != NULL){ //if entry is full...
      if (key == entry->key){ //check if key passed in matches key from an entry
       oldValue = entry->value; //store the old value
       entry->value = value; //replace the value with the value passed in 

       pthread_mutex_unlock(&locks[index]); //unlock row
       return oldValue; //return the old value
      }

     prev = entry; //set prev to current entry
     entry = prev->next; //move onto the next entry
   }
   
   //match not found, and haven't reached size yet so malloc space for new entry
   entry = (ts_entry_t*) malloc (1 * sizeof(ts_entry_t));  
   entry->key = key;  //sets key for new entry
   entry->value = value; //sets value for new entry
   entry->next = NULL; //set next pointer to null of new entry
   prev->next = entry; //insert the entry
   map->size++; //increment size

  pthread_mutex_unlock(&locks[index]); //unlock row
  return INT_MAX;
}


/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key) {

  // TODO
   unsigned int slot = (unsigned int) key; //cast key as unsigned int
   int index = slot % (map->capacity); //get index 

   pthread_mutex_lock(&locks[index]); //unlock row

   ts_entry_t *entry = map->table[index]; //get given entry
    int oldValue = 0;
   if (entry == NULL){ //if entry is null, return INT_MAX
     pthread_mutex_unlock(&locks[index]); //unlock row
     return INT_MAX;
   }
   else{
    ts_entry_t *prev;
    //ts_entry_t *temp;
    if (entry->next == NULL && key == entry->key){ //if this is the first and only element
      oldValue = entry->value; //store the value to return later
      free(entry); //free entry
      entry = NULL; //set to null
      map->table[index] = NULL;
      map->size--; //decrement size after deletion
      
      pthread_mutex_unlock(&locks[index]); //unlock row
      return oldValue; //return the old value
    }

    int firstItem = 1; //to keep track of the first element
    
    while (entry != NULL){ //if entry exists...
      if (key == entry->key){ //check if given key matches entry key
          if (firstItem == 1){ //if this is the first thing in list and there are elements after it
            oldValue = entry->value; //store the value to return later
            ts_entry_t *temp = entry->next;
            free(entry); //free
            entry = NULL; //set to null
            map->table[index] = temp;
            map->size--; //decrement size after deletion

            pthread_mutex_unlock(&locks[index]); //unlock row
            return oldValue;
          }
          else{ //the element to be deleted is in the middle of the list
            oldValue = entry->value; //store the value to return later
            prev->next = entry->next; //setting previous elements' next val to the element after deleted element
            free(entry); //free
            entry = NULL; //set to null
            map->size--; //decrement size after deletion

            pthread_mutex_unlock(&locks[index]); //unlock row
            return oldValue; //return the old value
          }
      }
      prev = entry;
      entry = prev->next;
      firstItem++;
    }
   }
   pthread_mutex_unlock(&locks[index]); //unlock row
   return INT_MAX;
}


/**
 * @return the load factor of the given map
 */
double lf(ts_hashmap_t *map) {
  return (double) map->size / map->capacity;
}

/**
 * Prints the contents of the map
 */
void printmap(ts_hashmap_t *map) {
  for (int i = 0; i < map->capacity; i++) {
    printf("[%d] -> ", i);
    ts_entry_t *entry = map->table[i];
    while (entry != NULL) {
      printf("(%d,%d)", entry->key, entry->value);
      if (entry->next != NULL)
        printf(" -> ");
      entry = entry->next;
    }
    printf("\n");
  }
}