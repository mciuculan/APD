#include <stdlib.h>
#include <pthread.h>
#include "genetic_algorithm_par.h"

int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

    // num of threads
    int number_of_threads = 0;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;

	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &number_of_threads, argc, argv)) {
		return 0;
	}

	pthread_t tid[number_of_threads];
	pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, number_of_threads);

    globals **global = malloc(sizeof(globals *) * number_of_threads);
	for (int i = 0; i < number_of_threads; i++) {
		global[i] = init(&barrier, i, number_of_threads, objects, object_count, sack_capacity, generations_count);
	}

	for (int i = 0; i < number_of_threads; i++) {
		pthread_create(&tid[i], NULL, run_genetic_algorithm, (void *)global[i]);
	}

	for (int i = 0; i < number_of_threads; i++) {
		pthread_join(tid[i], NULL);
	}

	pthread_barrier_destroy(&barrier);

    free(objects);

    return 0;
}
