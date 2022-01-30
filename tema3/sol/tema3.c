#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]) {
    int numtasks, rank, len;
 
    int dimension, commError;

    int noWorkersCurrentCluster;

    sscanf (argv[1],"%d",&dimension);
    sscanf (argv[2],"%d",&commError);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    // tag = 0 pt leader 0, 1 pt 1, 2 pt 2
    
    if (rank == 0) {
        FILE* inputFile = fopen("cluster0.txt", "r");
        int *ranks;
        int noWorkersForCluster1, *workersListForCluster1;
        int noWorkersForCluster2, *workersListForCluster2;
        
        // read input
        MPI_Status status;

        fscanf(inputFile, "%d", &noWorkersCurrentCluster);
        ranks = (int*)calloc(noWorkersCurrentCluster, sizeof(int));
        
        for (int i = 0; i < noWorkersCurrentCluster; i++)
            fscanf(inputFile, "%d", &ranks[i]);
            
        fclose(inputFile);
        // finish reading input
        
        if (!commError) {
            // send topology to 1
            MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);
        }
        
        // send topology to 2
        MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, 2, 0, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);

        if (!commError) {
            // receive topology from 1
            MPI_Recv(&noWorkersForCluster1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
            workersListForCluster1 = (int*)calloc(noWorkersForCluster1, sizeof(int));
            MPI_Recv(workersListForCluster1, noWorkersForCluster1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
        } else {
            // receive topology from 1 through 2
            MPI_Recv(&noWorkersForCluster1, 1, MPI_INT, 2, 1, MPI_COMM_WORLD, &status);
            workersListForCluster1 = (int*)calloc(noWorkersForCluster1, sizeof(int));
            MPI_Recv(workersListForCluster1, noWorkersForCluster1, MPI_INT, 2, 1, MPI_COMM_WORLD, &status);
        }

        // receive topology from 2
        MPI_Recv(&noWorkersForCluster2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);
        workersListForCluster2 = (int*)calloc(noWorkersForCluster2, sizeof(int));
        MPI_Recv(workersListForCluster2, noWorkersForCluster2, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);

        // send details to workers
        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            // let them know who the master is
            MPI_Send(&rank, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            // tell them about the number of workers to expect
            MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            // send current topology
            MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            // send topology for cluster 1
            MPI_Send(&noWorkersForCluster1, 1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            MPI_Send(workersListForCluster1, noWorkersForCluster1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            // send topology for cluster 2
            MPI_Send(&noWorkersForCluster2, 1, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            MPI_Send(workersListForCluster2, noWorkersForCluster2, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            // print source and destination
            printf("M(%d,%d)\n", rank, ranks[i]);
        }
        
        // print topology
        printf("0 -> ");
        printf("0:");
        for (int j = 0; j < noWorkersCurrentCluster; j++) {
            if (j < noWorkersCurrentCluster - 1) {
                printf("%d,",ranks[j]);
            } else {
                printf("%d ",ranks[j]);
            }
        }
        printf("1:");
        for (int j = 0; j < noWorkersForCluster1; j++) {
            if (j < noWorkersForCluster1 - 1) {
                printf("%d,",workersListForCluster1[j]);
            } else {
                printf("%d ",workersListForCluster1[j]);
            }
        }
        printf("2:");
        for (int j = 0; j < noWorkersForCluster2; j++) {
            if (j < noWorkersForCluster2 - 1) {
                printf("%d,",workersListForCluster2[j]);
            } else {
                printf("%d ",workersListForCluster2[j]);
            }
        }
        printf("\n");
        // finish printing

        // task 2

        // generate required vector
        int* v = (int*)calloc(dimension, sizeof(int));
        for (int i = 0; i < dimension; i++) {
            v[i] = i;
        }
        
        // distribute number of tasks evenly depending on the number of workers
        // of each cluster
        int forTask0, forTask1, forTask2;
        forTask0 = (int) floor(dimension / (noWorkersCurrentCluster + noWorkersForCluster1 + noWorkersForCluster2))
                                                                                            * noWorkersCurrentCluster; 
        forTask1 = (int) floor(dimension / (noWorkersCurrentCluster + noWorkersForCluster1 + noWorkersForCluster2))
                                                                                            * noWorkersForCluster1; 
        forTask2 = dimension - (forTask0 + forTask1);

        // calculate tasks for children
        int toBeSent = forTask0 / noWorkersCurrentCluster;
        
        // for each worker in cluster, send the tasks
        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            MPI_Send(&toBeSent, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            for (int j = i * toBeSent; j < (i + 1) * toBeSent; j++) {
                MPI_Send(&v[j], 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            }
        }

        if (!commError) {
            // send tasks for cluster 1
            MPI_Send(&forTask1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            for (int i = forTask0; i < forTask0 + forTask1; i++) {
                MPI_Send(&v[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            }
        } 
        if (commError)
        {
            // send tasks for cluster 1 through 2
            MPI_Send(&forTask1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            for (int i = forTask0; i < forTask0 + forTask1; i++) {
                MPI_Send(&v[i], 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            }
        }
        
        // send tasks for cluster 2
        MPI_Send(&forTask2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        for (int i = forTask0 + forTask1; i < dimension; i++) {
            MPI_Send(&v[i], 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        }

        // create vector for final solution
        int* final = (int*)calloc(dimension, sizeof(int));
        int counter = 0;

        // receive changes from each child
        // add them to the final solution
        int* modified0 = (int*)calloc(forTask0, sizeof(int));
        int k = 0;
        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            int aux;
            MPI_Recv(&aux, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD, &status);
            for (int j = 0; j < aux; j++) {
                MPI_Recv(&modified0[k], 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD, &status);
                k++;
            }
        }
        for (int i = 0; i < forTask0; i++) {
            final[counter] = modified0[i];
            counter++;
        }
        free(modified0);

        // receive changes from cluster 1 or from cluster 2
        // add them to the final solution
        int* modified1 = (int*)calloc(forTask1, sizeof(int));
        if (!commError) {
            MPI_Recv(modified1, forTask1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
        } else {
            MPI_Recv(modified1, forTask1, MPI_INT, 2, 20, MPI_COMM_WORLD, &status);
        }
        for (int i = 0; i < forTask1; i++) {
            final[counter] = modified1[i];
            counter++;
        }
        free(modified1);

        // receive changes from cluster 2
        // add them to the final solution
        int* modified2 = (int*)calloc(forTask2, sizeof(int));
        MPI_Recv(modified2, forTask2, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        for (int i = 0; i < forTask2; i++) {
            final[counter] = modified2[i];
            counter++;
        }
        free(modified2);
        
        // print final vector
        printf("Rezultat: ");
        for (int i = 0; i < dimension; i++) {
            printf("%d ", final[i]);
        }
        printf("\n");
        free(final);
        // finish printing final vector

    } else if (rank == 1) {
        FILE* inputFile = fopen("cluster1.txt", "r");
        int *ranks;
        int noWorkersForCluster0, *workersListForCluster0;
        int noWorkersForCluster2, *workersListForCluster2;

        MPI_Status status;

        // read input
        fscanf(inputFile, "%d", &noWorkersCurrentCluster);
        ranks = (int*)calloc(noWorkersCurrentCluster, sizeof(int));
        
        for (int i = 0; i < noWorkersCurrentCluster; i++)
            fscanf(inputFile, "%d", &ranks[i]);

        fclose(inputFile);
        // finish reading input

        if (!commError) {
            // receive topology from 0
            MPI_Recv(&noWorkersForCluster0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            workersListForCluster0 = (int*)calloc(noWorkersForCluster0, sizeof(int));
            MPI_Recv(workersListForCluster0, noWorkersForCluster0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

            // send topology to 0
            MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, 0, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);
        }

        // send topology to 2
        MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, 2, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 2);

        // receive topology from 2
        MPI_Recv(&noWorkersForCluster2, 1, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);
        workersListForCluster2 = (int*)calloc(noWorkersForCluster2, sizeof(int));
        MPI_Recv(workersListForCluster2, noWorkersForCluster2, MPI_INT, 2, 2, MPI_COMM_WORLD, &status);

        if (commError) {
            // receive topology from 0 through 2
            MPI_Recv(&noWorkersForCluster0, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            workersListForCluster0 = (int*)calloc(noWorkersForCluster0, sizeof(int));
            MPI_Recv(workersListForCluster0, noWorkersForCluster0, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        }

        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            // tell'em who the boss is
            MPI_Send(&rank, 1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            // number of workers
            MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);    
            // the workers       
            MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            // tell workers about cluster 0
            MPI_Send(&noWorkersForCluster0, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(workersListForCluster0, noWorkersForCluster0, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            // tell workers about cluster 1
            MPI_Send(&noWorkersForCluster2, 1, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            MPI_Send(workersListForCluster2, noWorkersForCluster2, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, ranks[i]);
        }
        
        // start printing
        printf("1 -> ");
        printf("0:");
        for (int j = 0; j < noWorkersForCluster0; j++) {
            if (j < noWorkersForCluster0 - 1) {
                printf("%d,",workersListForCluster0[j]);
            } else {
                printf("%d ",workersListForCluster0[j]);
            }
        }
        printf("1:");
        for (int j = 0; j < noWorkersCurrentCluster; j++) {
            if (j < noWorkersCurrentCluster - 1) {
                printf("%d,",ranks[j]);
            } else {
                printf("%d ",ranks[j]);
            }
        }
        printf("2:");
        for (int j = 0; j < noWorkersForCluster2; j++) {
            if (j < noWorkersForCluster2 - 1) {
                printf("%d,",workersListForCluster2[j]);
            } else {
                printf("%d ",workersListForCluster2[j]);
            }
        }
        printf("\n");
        // stop printing

        // task 2

        // receive from the first cluster the elements to be distributed
        int elementsToModify, *vectorToModify;
        if (!commError) {
            MPI_Recv(&elementsToModify, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        } else {
            MPI_Recv(&elementsToModify, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
        }
        
        vectorToModify = (int*)calloc(elementsToModify, sizeof(int));
        if (!commError) {
            for (int i = 0; i < elementsToModify; i++) {
                MPI_Recv(&vectorToModify[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            }
        }
        if (commError) {
            for (int i = 0; i < elementsToModify; i++) {
                MPI_Recv(&vectorToModify[i], elementsToModify, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            }
        }
        
        // get the number of elements that need to be computed for each child
        int toBeSent = elementsToModify / noWorkersCurrentCluster;
        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            MPI_Send(&toBeSent, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            // distribute tasks
            for (int j = i * toBeSent; j < (i + 1) * toBeSent; j++) {
                MPI_Send(&vectorToModify[j], 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            }
        }
        // create a new vector for the results
        free(vectorToModify);
        int k = 0;
        vectorToModify = (int*)calloc(elementsToModify, sizeof(int));
        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            int aux;
            MPI_Recv(&aux, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD, &status);
            // add the results to the new vector
            for (int j = 0; j < aux; j++) {
                MPI_Recv(&vectorToModify[k], 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD, &status);
                k++;
            }
        }
    
        if (!commError) {
            // send final results to cluster 0
            MPI_Send(vectorToModify, elementsToModify, MPI_INT, 0, 0, MPI_COMM_WORLD);
        } else {
            // send final results to cluster 0 through 2
            MPI_Send(&elementsToModify, 1, MPI_INT, 2, 20, MPI_COMM_WORLD);
            
            MPI_Send(vectorToModify, elementsToModify, MPI_INT, 2, 20, MPI_COMM_WORLD);
        }

    } else if (rank == 2) {

        FILE* inputFile = fopen("cluster2.txt", "r");
        int *ranks;
        int noWorkersForCluster0, *workersListForCluster0;
        int noWorkersForCluster1, *workersListForCluster1;
        
        MPI_Status status;

        // read input
        fscanf(inputFile, "%d", &noWorkersCurrentCluster);
        ranks = (int*)calloc(noWorkersCurrentCluster, sizeof(int));
        
        for (int i = 0; i < noWorkersCurrentCluster; i++)
            fscanf(inputFile, "%d", &ranks[i]);
        
        fclose(inputFile);
        // finish reading input

        // receive topology from 0
        MPI_Recv(&noWorkersForCluster0, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        workersListForCluster0 = (int*)calloc(noWorkersForCluster0, sizeof(int));
        MPI_Recv(workersListForCluster0, noWorkersForCluster0, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        
        // receive topology from 1
        MPI_Recv(&noWorkersForCluster1, 1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);
        workersListForCluster1 = (int*)calloc(noWorkersForCluster1, sizeof(int));
        MPI_Recv(workersListForCluster1, noWorkersForCluster1, MPI_INT, 1, 1, MPI_COMM_WORLD, &status);

        // send current topology to 0
        MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, 0, 2, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 0);

        // send current topology to 1
        MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
        MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, 1, 2, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, 1);

        if (commError) {
            // send cluster 0 topology to 1
            MPI_Send(&noWorkersForCluster0, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            MPI_Send(workersListForCluster0, noWorkersForCluster0, MPI_INT, 1, 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 1);

            // send cluster 1 topology to 0
            MPI_Send(&noWorkersForCluster1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            MPI_Send(workersListForCluster1, noWorkersForCluster1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, 0);
        }
        
        // send topologies to children
        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            // tell them who the master is
            MPI_Send(&rank, 1, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            // tell them master's topology
            MPI_Send(&noWorkersCurrentCluster, 1, MPI_INT, ranks[i], 2, MPI_COMM_WORLD); 
            MPI_Send(ranks, noWorkersCurrentCluster, MPI_INT, ranks[i], 2, MPI_COMM_WORLD);
            // tell them cluster 0's topology
            MPI_Send(&noWorkersForCluster0, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            MPI_Send(workersListForCluster0, noWorkersForCluster0, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            // tell them cluster 1's topology
            MPI_Send(&noWorkersForCluster1, 1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            MPI_Send(workersListForCluster1, noWorkersForCluster1, MPI_INT, ranks[i], 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, ranks[i]);
        }
        
        // start printing topology
        printf("2 -> ");
        printf("0:");
        for (int j = 0; j < noWorkersForCluster0; j++) {
            if (j < noWorkersForCluster0 - 1) {
                printf("%d,",workersListForCluster0[j]);
            } else {
                printf("%d ",workersListForCluster0[j]);
            }
        }
        printf("1:");
        for (int j = 0; j < noWorkersForCluster1; j++) {
            if (j < noWorkersForCluster1 - 1) {
                printf("%d,",workersListForCluster1[j]);
            } else {
                printf("%d ",workersListForCluster1[j]);
            }
        }
        printf("2:");
        for (int j = 0; j < noWorkersCurrentCluster; j++) {
            if (j < noWorkersCurrentCluster - 1) {
                printf("%d,",ranks[j]);
            } else {
                printf("%d ",ranks[j]);
            }
        }
        printf("\n");
        // end printing topology

        // task 2

        if (commError) {
            // receive from first and send to second
            int elementsToModify, *vectorToModify;
            MPI_Recv(&elementsToModify, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            vectorToModify = (int*)calloc(elementsToModify, sizeof(int));
            for (int i = 0; i < elementsToModify; i++) {
                MPI_Recv(&vectorToModify[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            }
            MPI_Send(&elementsToModify, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            for (int i = 0; i < elementsToModify; i++) {
                MPI_Send(&vectorToModify[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            }
        }

        // receive from the first cluster the elements to be distributed
        int elementsToModify, *vectorToModify;
        MPI_Recv(&elementsToModify, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        vectorToModify = (int*)calloc(elementsToModify, sizeof(int));
        for (int i = 0; i < elementsToModify; i++) {
            MPI_Recv(&vectorToModify[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }

        // get the number of elements that need to be computed for each child 
        int toBeSent = (int) floor(elementsToModify / noWorkersCurrentCluster);
        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            MPI_Send(&toBeSent, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            // distribute tasks
            for (int j = i * toBeSent; j < (i + 1) * toBeSent; j++) {
                MPI_Send(&vectorToModify[j], 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD);
            }
        }

        // create a new vector for the results
        free(vectorToModify);
        int k = 0;
        vectorToModify = (int*)calloc(elementsToModify, sizeof(int));
        for (int i = 0; i < noWorkersCurrentCluster; i++) {
            int aux;
            MPI_Recv(&aux, 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD, &status);
            for (int j = 0; j < aux; j++) {
                // add the results to the new vector
                MPI_Recv(&vectorToModify[k], 1, MPI_INT, ranks[i], 0, MPI_COMM_WORLD, &status);
                k++;
            }
        }

        // send cluster1 final results to cluster 0
        if (commError) {
            int aux;
            MPI_Recv(&aux, 1, MPI_INT, 1, 20, MPI_COMM_WORLD, &status);
            int *vector = (int*)calloc(aux, sizeof(int));
            MPI_Recv(vector, aux, MPI_INT, 1, 20, MPI_COMM_WORLD, &status);
            // MPI_Send(&aux, 1, MPI_INT, 0, 20, MPI_COMM_WORLD);
            MPI_Send(vector, aux, MPI_INT, 0, 20, MPI_COMM_WORLD);
        }
        
        // send final results to 0
        MPI_Send(vectorToModify, elementsToModify, MPI_INT, 0, 0, MPI_COMM_WORLD);
        
    } else {
        int leader;
        int noWorkersForCluster0, *workersListForCluster0;
        int noWorkersForCluster1, *workersListForCluster1;
        int noWorkersForCluster2, *workersListForCluster2;

        MPI_Status status;

        // find out who is the leader
        MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // receive from leader the whole topology
        // get info about the first cluster
        MPI_Recv(&noWorkersForCluster0, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        workersListForCluster0 = (int*)calloc(noWorkersForCluster0, sizeof(int));
        MPI_Recv(workersListForCluster0, noWorkersForCluster0, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);

        // get info about the second cluster
        MPI_Recv(&noWorkersForCluster1, 1, MPI_INT, leader, 1, MPI_COMM_WORLD, &status);
        workersListForCluster1 = (int*)calloc(noWorkersForCluster1, sizeof(int));
        MPI_Recv(workersListForCluster1, noWorkersForCluster1, MPI_INT, leader, 1, MPI_COMM_WORLD, &status);

        // get info about the third cluster
        MPI_Recv(&noWorkersForCluster2, 1, MPI_INT, leader, 2, MPI_COMM_WORLD, &status);
        workersListForCluster2 = (int*)calloc(noWorkersForCluster2, sizeof(int));
        MPI_Recv(workersListForCluster2, noWorkersForCluster2, MPI_INT, leader, 2, MPI_COMM_WORLD, &status);

        // current worker starts printing
        printf("%d -> ", rank);

        printf("0:");
        for (int j = 0; j < noWorkersForCluster0; j++) {
            if (j < noWorkersForCluster0 - 1) {
                printf("%d,",workersListForCluster0[j]);
            } else {
                printf("%d ",workersListForCluster0[j]);
            }
        }

        printf("1:");
        for (int j = 0; j < noWorkersForCluster1; j++) {
            if (j < noWorkersForCluster1 - 1) {
                printf("%d,",workersListForCluster1[j]);
            } else {
                printf("%d ",workersListForCluster1[j]);
            }
        }

        printf("2:");
        for (int j = 0; j < noWorkersForCluster2; j++) {
            if (j < noWorkersForCluster2 - 1) {
                printf("%d,",workersListForCluster2[j]);
            } else {
                printf("%d ",workersListForCluster2[j]);
            }
        }
        printf("\n");
        // current worker stops printing
        
        // task 2
        int elementsToModify, *vectorToModify;
        // receive number of elements to modify
        MPI_Recv(&elementsToModify, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        
        vectorToModify = (int*)calloc(elementsToModify, sizeof(int));

        // receive vector of elements to modify
        for (int i = 0; i < elementsToModify; i++) {
            MPI_Recv(&vectorToModify[i], 1, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        }

        // compute changes to vector
        for (int i = 0; i < elementsToModify; i++) {
            vectorToModify[i] *= 2;
        }

        // tell leader how many elements to expect
        MPI_Send(&elementsToModify, 1, MPI_INT, leader, 0, MPI_COMM_WORLD);

        // send modified elements to leader
        for (int j = 0; j < elementsToModify; j++) {
            MPI_Send(&vectorToModify[j], 1, MPI_INT, leader, 0, MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
}