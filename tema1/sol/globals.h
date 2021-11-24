#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct globals
{
    pthread_barrier_t *barrier;
    int thread_id;
    int no_of_threads;
    const sack_object *objects;
    int object_count;
    int sack_capacity;
    int generations_count;
	individual *current_generation;
    individual *next_generation;
} globals;

#endif
