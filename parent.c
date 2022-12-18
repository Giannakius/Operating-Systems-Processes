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

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#include "shared_memory.h"

void child (int ,int ,int ,int , int  , int*  , SharedMemory , sem_t** ,void* ,void* ,void*);

int main(int argc, char *argv[]) {
    
    // Check the arguements to be the right number of input
    if (argc != 5) {
        perror("You gave me wrong number of input's. \n");
        exit(1);
    }

    // Open file
    FILE* fptr;
    fptr = fopen(argv[1],"r");
    if (fptr == NULL) {
        perror("The file you gave me is empty. \n");
        exit(1);
    }
    
    // Number of requests from every child
    int number_of_requests = atoi(argv[4]);

    // Create childs
    int Number_of_Childs = atoi(argv[3]);


    // Get number of Sum_of_Lines
    int Sum_of_Lines = 0;
    for (char i = getc(fptr); i != EOF; i = getc(fptr)) {
        if (i == '\n') {
            Sum_of_Lines++;
        }
    }
    Sum_of_Lines++;
    fseek(fptr,0,SEEK_SET);
    
    // Segmentation_Degree // Bathmos katatmisis
    int Segmentation_Degree = atoi(argv[2]);

    // Check if segmentation rate is big
    if (Segmentation_Degree > MAX_SEGMENTATION_RATE) {
        perror("Segmentation rate is bigger than more Max segmentation rate\n");
        exit(1);
    }
 
    // Number of segments
    int Num_of_Segments ; 
    if (Sum_of_Lines % Segmentation_Degree == 1) {
        Num_of_Segments = Sum_of_Lines / Segmentation_Degree + 1 ; 
    }
    
    else{
        Num_of_Segments = Sum_of_Lines / Segmentation_Degree ; 
    }

    // Check if file is small
    if (Sum_of_Lines < 1000) {
        perror("You gave me a small text file \n");
        exit(1);
    }
    
    // Check if segmentation rate is bigger more than Sum_of_Lines
    if (Segmentation_Degree > Sum_of_Lines) {
        perror("Lines are bigger more than segmentation rate\n");
        exit(1);
    }
    
    // Create shared memory
    int shmid;
    SharedMemory S_M;
    if((shmid = shmget(IPC_PRIVATE, sizeof(*S_M), 0666 | IPC_CREAT)) == -1){
        perror("Failed to create shared memory");
        exit(1);
    }

    // Attach memory segment
    if((S_M = shmat(shmid, NULL, 0)) == (void*)-1){
        perror("Failed to attach memory segment");
        exit(1);
    }
    
    // Parent is open to request
    sem_t *request_parent = sem_open("request_parent", O_CREAT, SEM_PERMS, 1);
    sem_unlink("request_parent");
    if (request_parent == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }

    // Child ready to give a segment
    sem_t *child_ready = sem_open("child_ready", O_CREAT, SEM_PERMS, 0);
    sem_unlink("child_ready");
    if (child_ready == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }

    // Parent upload text segment to shared memory
    sem_t *parent_answer = sem_open("parent_answer", O_CREAT, SEM_PERMS, 0);
    sem_unlink("parent_answer");
    if (parent_answer == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }
    
    // Create an array of semaphorshes one per segment
    sem_t ** segment_semaphores = malloc(Num_of_Segments*sizeof(*segment_semaphores));
 
    for (int i = 0; i < Num_of_Segments; i++) {

        char buffer[120];
        snprintf(buffer, sizeof(buffer), "%s%d", "segment", i);
        
        segment_semaphores[i] = sem_open(buffer, O_CREAT, SEM_PERMS, 1);
        sem_unlink(buffer);
        if (segment_semaphores[i] == SEM_FAILED) {
            fprintf(stderr, "%dth semaphore open fail\n", i);
            exit(1);
        }
    }

    
    // How many children is ready to read per segment   
    int *ready_children = malloc(Num_of_Segments*sizeof(int));  
    for(int i = 0;i < Num_of_Segments; i++){ 
        ready_children[i] = 0;  
    }   

    // 0 children have finished their requests
    S_M->finished = 0;

    int *pid = malloc(Number_of_Childs*sizeof(int));
    for(int i = 0; i<Number_of_Childs ; i++) {
        
        pid[i] = fork();
        
        if(pid[i] < 0) {
            perror("Fork failed");
            exit(1);
        }

        // Children code
        if (pid[i] == 0) {
            child(number_of_requests , Num_of_Segments , i , Number_of_Childs , Segmentation_Degree , ready_children , S_M, segment_semaphores , request_parent , parent_answer , child_ready);
        }
    }

    // Until all children fineshed their requests
    while (S_M->finished < Number_of_Childs) {
        
        // wait until child upload desired segment
        if(sem_wait(child_ready) < 0) {
            perror("sem wait child_ready semaphore at parent failed");
            exit(1);
        }

        // Finished parent programm
        if (S_M->finished == Number_of_Childs) break;

        // sleep for 20ms
        usleep(20000);

        //  child upload this segment
        int request_segment = S_M->request_segment;

        // find this segment
        char line[MAX_LINE];

        // Find desired segment
        for(int i = 0; i < request_segment; i ++) {
            for(int j = 0 ; j < Segmentation_Degree; j++) {
                fgets(line,MAX_LINE,fptr);
            }
        }
        // Upload desired segment to shared memory
        for(int j = 0 ; j < Segmentation_Degree; j++) {
                fgets(line,MAX_LINE,fptr);
                strcpy(S_M->buffer[j],line);
        }
        fseek(fptr,0,SEEK_SET);


        if(sem_post(parent_answer) < 0) {
            perror("sem_post parent answer at parent failed");
            exit(1);
        }
               
    }
    
    free(ready_children);
    free(pid);
    free(segment_semaphores);
    return (0);
}