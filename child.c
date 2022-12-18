#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <sys/shm.h>

#include "shared_memory.h"

void child (int number_of_requests,int num_of_segments,int i,int N, int Segmentation_Degree , int* ready_children , SharedMemory s_m,
 sem_t** segment_semaphores,void* request_parent,void* parent_answer,void* child_ready ){
    
    
    char buffer[120];
    snprintf(buffer, sizeof(buffer), "Log_%s%d.txt", "child", i);
    FILE *f = fopen(buffer, "w");

    // clock for request and for answer
    clock_t request_clock;
    clock_t answer_clock;

    // Number of requests
    int temp_request = 0;

    

    while (temp_request<number_of_requests) {
        
        int a,b; // a = desired segment , b = desired line
        if (temp_request == 0) {
            srand(time(NULL) ^ (getpid()<<16));
            a = rand()%num_of_segments;
            
        } else {
            int p = rand()%10 + 1; // p = propability
            if (p > 7) a = rand()%num_of_segments;
        }
        b = rand()%Segmentation_Degree;
        
        // clock starting when child find her line                
        request_clock = clock();

        // wait all children in segment semaphore (not the first time (sem_val = 0))
        if(sem_wait(segment_semaphores[a]) < 0) {
            fprintf(stderr, "error in wait semaphore %d", a);
            exit(1);
        }

        // children of segment a is ready
        ready_children[a]++;

        // FiFo logic.
        if(ready_children[a] == 1) {

            if(sem_wait(request_parent) < 0) {
                perror("sem_wait failed on child");
                exit(1);
            }

            // Upload request segment to shared memory
            s_m->request_segment = a;

            // request ends
            request_clock = clock() - request_clock;

            // clock starting when child make request
            answer_clock = clock();

            // child ready
            if(sem_post(child_ready) < 0){
                perror("sem_post child_ready failed on child");
                exit(1);
            }

            // wait until parent write to shared memory    
            if(sem_wait(parent_answer) < 0) {
                perror("sem_wait parent_answer failed on child");
                exit(1);
            }
        }
        
        // Enter to reading section
        if(sem_post(segment_semaphores[a]) < 0){
            fprintf(stderr, "error in post semaphore %d", a);
            exit(1);
        }
        
        
        // Reading section
        if(a == s_m->request_segment) {

            // child reading requesting line
            char reading_line[MAX_LINE];
            strcpy(reading_line,s_m->buffer[b]);
            
            // child finished one request
            temp_request++;
            
            // child get the answer
            answer_clock = clock() - answer_clock;

            fprintf(f,"Visited <Segment,Line> = <%d,%d>\n Request Time = %.15ld , Answer Time = %.15ld  \n %s \n",a,b,request_clock,answer_clock,reading_line);
        }

        // Leaving reading section
        if(sem_wait(segment_semaphores[a]) < 0) {
            fprintf(stderr, "error in wait semaphore %d", a);
            exit(1);
        }
        
        // Child of segment a finished
        ready_children[a]--;

        // Change segment
        if(ready_children[a] == 0) {
            
            // Change segment
            if(sem_post(request_parent) < 0){
                perror("sem post request_parent failed on child");
                exit(1);
            }
        }

        // Next request
        if(sem_post(segment_semaphores[a]) < 0) {
            fprintf(stderr, "error in post semaphore %d", a);
            exit(1);
        }
    }
    
    // This child finished his requests
    s_m->finished++;
    
    // Conclusion to finished the parent programm
    if(s_m->finished == N) {
        if(sem_post(child_ready) < 0) {
            perror("error in sem post child ready in child");
            exit(1);
        }
    } 
    // Close all semaphorses
    if(sem_close(request_parent) < 0 ) {
        perror("request parent sem_close error");
        exit(1);
    }    
    if(sem_close(parent_answer) < 0 ) {
        perror("request parent sem_close error");
        exit(1);
    }    
    if(sem_close(child_ready) < 0 ) {
        perror("request parent sem_close error");
        exit(1);
    }    
    for(int i = 0; i < num_of_segments;i++) {
        if(sem_close(segment_semaphores[i]) < 0 ) {
        perror("segment semaphoreses sem_close error");
        exit(1);
    }    
    }
    exit(0); 
}