#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/shm.h>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <cstring>
#include "shared_memory.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

// find a random Segment with propability 0.7 for the same segment and 0.3 for different
int Rand_Segment(int segment , int Num_Of_Segments)
{
    int new_segment = segment;
    int PITHANOTHTA = 1 + (rand() % 10);    // pithanotita 1 - 10 
    
    // An tyxei 1-2-3-4-5-6-7 diladi 70% tote menoyme sto idio segment
    if (PITHANOTHTA<=7){
        cout << "Idio segment " ; 
    }
    else { // An tyxei 8-9-10 , diladi 30% pithanotita tote pame se opoiodipote segment ektos apo ayto poy eimastan
        while (new_segment == segment){
            new_segment = 1 + (rand() % Num_Of_Segments);
        }
        cout << "allo segment " ; 
    }
    cout << new_segment << endl;
    return new_segment ;
}

void child(int nlines, int N, int i, Shared_memory semlock, sem_t* requestProducer, sem_t* answerProducer, sem_t* answerConsumer){

    clock_t t;
    int sum_time = 0;   // Total time for N requests to server

    // Every child begins with requests
    int requests = 0;
    while(requests < N){
        printf("creating child %d\n",i);
        // Wait for producer to finish initation of all children
        if(sem_wait(requestProducer) < 0){
            perror("sem_wait failed on parent");
            exit(EXIT_FAILURE);
        }
        printf("DONE\n");
        if(requests >= N){
            return;
        }
        
        // Rand seed and generate random line
        unsigned int curtime = time(NULL);
        srand((unsigned int) curtime - getpid());
        int line = rand() % nlines + 1;

        // Write to shared memory segment
        semlock->temp_Segment = line;

        t = clock();    // Begin timing
        printf("counting time\n");
        // Consumer answers
        if(sem_post(answerConsumer) < 0){
            perror("sem_post failed on parent");
            exit(EXIT_FAILURE);
        }

        // Wait for answer from producer
        if(sem_wait(answerProducer) < 0){
            perror("sem_wait failed on parent");
            exit(EXIT_FAILURE);
        }

        t = clock() - t;    // End timing
        sum_time += t;
        
        requests++;
        if(requests == N){
            semlock->finished = 1;
        }

        // Consumer answers
        if(sem_post(answerConsumer) < 0){
            perror("sem_post failed on parent");
            exit(EXIT_FAILURE);
        }
        
    }

    double time_taken = ((double)sum_time)/CLOCKS_PER_SEC; // calculate the total elapsed time
    printf("Child %d waited on average %.15lf seconds for server response\n", getpid(), time_taken/N);

    // Close semaphores used by this child (if not done, leaks are present)
    if(sem_close(requestProducer) < 0){
        perror("sem_close(0) failed on child");
        exit(EXIT_FAILURE);
    }

    if(sem_close(answerProducer) < 0){
        perror("sem_close(1) failed on child");
        exit(EXIT_FAILURE);
    }

    if(sem_close(answerConsumer) < 0){
        perror("sem_close(2) failed on child");
        exit(EXIT_FAILURE);
    }

    return;
}