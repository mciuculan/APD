#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "genetic_algorithm_par.h"

// init auxilliary struct
globals *init(  pthread_barrier_t *barrier, int thread_id, int no_of_threads,
                const sack_object *objects,
                int object_count,
                int sack_capacity,
				int generations_count) {
    globals *global = malloc(sizeof(globals));
    global->barrier = barrier;
    global->no_of_threads = no_of_threads;
    global->object_count = object_count;
    global->objects = objects;
    global->sack_capacity = sack_capacity;
    global->thread_id = thread_id;
    global->generations_count = generations_count;
    return global;
}

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *number_of_threads, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 3) {
		fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count number_of_threads\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);

    *number_of_threads = (int) strtol(argv[3], NULL, 10);

	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity)
{
	int weight;
	int profit;

	for (int i = 0; i < object_count; ++i) {
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	int i;
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {
		int first_count = 0, second_count = 0;

		for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i) {
			first_count += first->chromosomes[i];
			second_count += second->chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0) {
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	} else {
		// for odd-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i) {
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

void *run_genetic_algorithm(void *arg)
{
	int count1, count2, cursor;
	globals *global = (globals *) arg;
	individual *current_generation = (individual*) calloc(global->object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(global->object_count, sizeof(individual));
	individual *tmp = NULL;
	long id = global->thread_id;
    int start1, start2;
    int end1par, end1impar, end2par;
	count1 = global->object_count * 3 / 10;
	count2 = global->object_count * 2 / 10;

	start1 = id * (double)count1 / global->no_of_threads;
	if ((id + 1) * (double)count1 / global->no_of_threads < count1) {
		end1par = (id + 1) * (double)count1 / global->no_of_threads;
		end1impar = (id + 1) * (double)count1 / global->no_of_threads;
	} else {
		end1par = count1;
		end1impar = count1 - 1;
	}

	start2 = id * (double)count2 / global->no_of_threads;
	if ((id + 1) * (double)count2 / global->no_of_threads < count2) {
		end2par = (id + 1) * (double)count2 / global->no_of_threads;
	} else {
		end2par = count2;
	}

        // set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = 0; i < global->object_count; ++i) {
		current_generation[i].fitness = 0;
		current_generation[i].chromosomes = (int*) calloc(global->object_count, sizeof(int));
		current_generation[i].chromosomes[i] = 1;
		current_generation[i].index = i;
		current_generation[i].chromosome_length = global->object_count;

		next_generation[i].fitness = 0;
		next_generation[i].chromosomes = (int*) calloc(global->object_count, sizeof(int));
		next_generation[i].index = i;
		next_generation[i].chromosome_length = global->object_count;
	}

	// iterate for each generation
	for (int k = 0; k < global->generations_count; ++k) {
		cursor = 0;

		// compute fitness and sort by it
		compute_fitness_function(global->objects, current_generation, global->object_count, global->sack_capacity);
		qsort(current_generation, global->object_count, sizeof(individual), cmpfunc);

		// keep first 30% children (elite children selection)
		for (int i = start1; i < end1par; ++i) {
			copy_individual(current_generation + i, next_generation + i);
		}
		pthread_barrier_wait (global->barrier);
		cursor = count1; // nu e nev neaparat

		// mutate first 20% children with the first version of bit string mutation
		for (int i = start2; i < end2par; ++i) {
			copy_individual(current_generation + i, next_generation + cursor + i);
			mutate_bit_string_1(next_generation + cursor + i, k);
		}
		pthread_barrier_wait (global->barrier);
		cursor += count2;

		// mutate next 20% children with the second version of bit string mutation
		for (int i = start2; i < end2par; ++i) {
			copy_individual(current_generation + i + count2, next_generation + cursor + i);
			mutate_bit_string_2(next_generation + cursor + i, k);
		}
		pthread_barrier_wait (global->barrier);
		cursor += count2;

		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)
		if (count1 % 2 == 1) {
			copy_individual(current_generation + global->object_count - 1, next_generation + cursor + count1 - 1);
			for (int i = start1; i < end1impar; i += 2) {
				crossover(current_generation + i, next_generation + cursor + i, k);
			}
		} else {
			for (int i = start1; i < end1par; i += 2) {
				crossover(current_generation + i, next_generation + cursor + i, k);
			}
		}
		pthread_barrier_wait (global->barrier);

		// switch to new generation
		tmp = current_generation;
		current_generation = next_generation;
		next_generation = tmp;

		for (int i = 0; i < global->object_count; ++i) {
			current_generation[i].index = i;
		}

		if (k % 5 == 0) {
			if (global->thread_id == 0) {
				print_best_fitness(current_generation);
			}
		}
	}

	compute_fitness_function(global->objects, current_generation, global->object_count, global->sack_capacity);
	qsort(current_generation, global->object_count, sizeof(individual), cmpfunc);
	if (global->thread_id == 0) {
		print_best_fitness(current_generation);
	}

	// free resources for old generation
	free_generation(current_generation);
	free_generation(next_generation);

	// free resources
	free(current_generation);
	free(next_generation);
	pthread_exit(NULL);
}
