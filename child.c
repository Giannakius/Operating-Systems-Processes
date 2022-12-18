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
        
        int temp_segment , temp_line;
        if (temp_request == 0) {
            srand(time(NULL) ^ (getpid()<<16));
            temp_segment = rand()%num_of_segments;
            
        } else {
            int propability;
            // there is 30% propability to go to some other segment and 70% to stay at the same segment.
            propability = rand() % 10 + 1;
            if (propability > 7) {
                temp_segment = rand ( ) % num_of_segments;
            }
        }
        temp_line = rand() % Segmentation_Degree;
        
        // clock starting when child find her line                
        request_clock = clock();

        // wait all children in segment semaphore (not the first time (sem_val = 0))
        if(sem_wait(segment_semaphores[temp_segment]) < 0) {
            fprintf(stderr, "error in wait semaphore %d", temp_segment);
            exit(1);
        }

        // children of segment a is ready
        ready_children[temp_segment]++;

     
        if(ready_children[temp_segment] == 1) {

            if(sem_wait(request_parent) < 0) {
                perror("sem_wait failed on child");
                exit(1);
            }

            // Upload request segment to shared memory
            s_m->request_segment = temp_segment;

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
        if(sem_post(segment_semaphores[temp_segment]) < 0){
            fprintf(stderr, "error in post semaphore %d", temp_segment);
            exit(1);
        }
        
        
        // Reading section
        if(temp_segment == s_m->request_segment) {

            // child reading requesting line
            char reading_line[MAX_LINE];
            strcpy(reading_line,s_m->buffer[temp_line]);
            
            // child finished one request
            temp_request++;
            
            // child get the answer
            answer_clock = clock() - answer_clock;

            fprintf(f,"Visited <Segment,Line> = <%d,%d>\n Request Time = %.15ld , Answer Time = %.15ld  \n %s \n" , temp_segment , temp_line , request_clock , answer_clock , reading_line );
        }

        // Leaving reading section
        if(sem_wait(segment_semaphores[temp_segment]) < 0) {
            fprintf(stderr, "error in wait semaphore %d", temp_segment);
            exit(1);
        }
        
        // Child of segment a finished
        ready_children[temp_segment]--;

        // Change segment
        if(ready_children[temp_segment] == 0) {
            
            // Change segment
            if(sem_post(request_parent) < 0){
                perror("sem post request_parent failed on child");
                exit(1);
            }
        }

        // Next request
        if(sem_post(segment_semaphores[temp_segment]) < 0) {
            fprintf(stderr, "error in post semaphore %d", temp_segment);
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