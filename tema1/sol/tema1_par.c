#include <stdlib.h>
#include <pthread.h>
#include "genetic_algorithm_par.h"

int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;
	int i;
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

    individual *current_generation = (individual *)calloc(object_count, sizeof(individual));
    individual *next_generation = (individual *)calloc(object_count, sizeof(individual));

	pthread_t tid[number_of_threads];
	pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, number_of_threads);

    globals **global = malloc(sizeof(globals *) * number_of_threads);
	for (i = 0; i < number_of_threads; i++) {
		global[i] = init(&barrier, i, number_of_threads, objects, object_count, sack_capacity, generations_count,
										current_generation, next_generation);
	}

	for (i = 0; i < number_of_threads; i++) {
		pthread_create(&tid[i], NULL, run_genetic_algorithm, (void *)global[i]);
	}

	for (i = 0; i < number_of_threads; i++) {
		pthread_join(tid[i], NULL);
	}

	pthread_barrier_destroy(&barrier);
	if (next_generation != NULL) {
		free_generation(next_generation);
		free(next_generation);
	}

	if (current_generation != NULL) {
		free_generation(current_generation);
		free(current_generation);
	}

    free(objects);
	for (i = 0; i < number_of_threads; i++) {
		free(global[i]);
	}
	free(global);


    return 0;
}
