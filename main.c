
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts_hashmap.h"

/**
* Globals
**/
ts_hashmap_t *map; //hashmap

void *testRoutine(){
	for (int i = 0; i < 5; i++){ //test loops 5 times
		int action = (rand() % 3) + 1; //generate an action for threads to do

		if (action == 1){ //do a put
		    int r = rand() % 100; //get a random number
			put(map, r, r);
		}
		else if (action == 2) { //do a get
			int r = rand() % 100;
			get(map, r);
		}
		else if (action == 3){ //do a del
			int r = rand() % 100;
       		del(map, r);
		}
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s <num threads> <hashmap capacity>\n", argv[0]);
		return 1;
	}
	
  	srand(time(NULL)); 
	int num_threads = atoi(argv[1]);
	int capacity = (unsigned int) atoi(argv[2]);

	map = initmap(capacity);

	// allocate space to hold threads
  	pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));

	for (int i = 0; i < num_threads; i++) { //spawn off threads
    	pthread_create(&threads[i], NULL, testRoutine, NULL);
  	}

	for (int i = 0; i < num_threads; i++){ //join threads
		pthread_join(threads[i], NULL);
	}

	printmap(map);

	for (int i = 0; i < capacity; i++){
		ts_entry_t *prev; //prev is for cycling through entries
		ts_entry_t *ent; //the entry

		ent = map->table[i];
		
		while (ent != NULL){
			prev = ent; //prev and ent point to the same thing
			ent = ent->next; //move ent along
			free(prev); //free prev
			prev = NULL; //prev set to NULL
		}
		map->table[i] = NULL;
	}

	free(map->table); //free up space of table
	map->table = NULL; //defensive programming
	free(map); //free up space for map
	map = NULL; //defensive programming

	free(threads); //free threads
	threads = NULL;

	for (int i = 0; i < capacity; i++){ //destroy mutexes
		pthread_mutex_destroy(&locks[i]);
	}

	free(locks); //free locks array
	locks = NULL;

	return 0;
}

