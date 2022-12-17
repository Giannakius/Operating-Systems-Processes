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

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

int main(int argc, char *argv[]) {
    
    // Check if the arguements is correct
    if (argc != 5) {
        perror("Give me a wrong inputs\n");
        exit(1);
    }

    // Open file and check if its empty
    FILE* fptr;
    fptr = fopen(argv[1],"r");
    if (fptr == NULL) {
        perror("You gave me wrong file\n");
        exit(1);
    }

    // Segmentation Degree // Bathmos Katatmisis
    int Segmentation_Degree = atoi(argv[2]);

    // Check if segmentation rate is big
    if (Segmentation_Degree > MAX_SEGMENTATION_DEGREE) {
        perror("Segmentation rate is bigger than more Max segmentation rate\n");
        exit(1);
    }

 
    // Number of childs
    int Number_of_Childs = atoi(argv[3]);

    // Number of requests from every child
    int number_of_requests = atoi(argv[4]);



    // Get the number of lines of .txt
    int lines = 0;
    for (char i = getc(fptr); i != EOF; i = getc(fptr)) {
        if (i == '\n') lines++;
    }
    lines++;
    fseek(fptr,0,SEEK_SET);
    

    // Number of segments
    int num_of_segments; 
    if (lines%Segmentation_Degree==1) {
        num_of_segments = lines/Segmentation_Degree + 1 ;
    }
    else {
        num_of_segments = lines/Segmentation_Degree;
    }



    // Check if file is small
    if (lines < 1000) {
        perror("You gave me a small text file\n");
        exit(1);
    }
    
    // Check if segmentation rate is bigger more than lines
    if (Segmentation_Degree > lines) {
        perror("Lines are bigger more than segmentation rate\n");
        exit(1);
    }
    
    // Create shared memory
    int shmid;
    SharedMemory sm;
    if((shmid = shmget(IPC_PRIVATE, sizeof(*sm), 0666 | IPC_CREAT)) == -1){
        perror("Failed to create shared memory");
        exit(1);
    }

    // Attach memory segment
    if((sm = shmat(shmid, NULL, 0)) == (void*)-1){
        perror("Failed to attach memory segment");
        exit(1);
    }
    
    // Parent is open to request
    sem_t *Parent_Ready_For_Request = sem_open("Parent_Ready_For_Request", O_CREAT, SEM_PERMS, 1);
    sem_unlink("Parent_Ready_For_Request");
    if (Parent_Ready_For_Request == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }

    // Child ready to give a segment
    sem_t *Child_Ready_For_Segment = sem_open("Child_Ready_For_Segment", O_CREAT, SEM_PERMS, 0);
    sem_unlink("Child_Ready_For_Segment");
    if (Child_Ready_For_Segment == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }

    // Parent upload text segment to shared memory
    sem_t *Parent_Answer_Request = sem_open("Parent_Answer_Request", O_CREAT, SEM_PERMS, 0);
    sem_unlink("Parent_Answer_Request");
    if (Parent_Answer_Request == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }
    
    // Create an array of semaphoreshes one per segment
    sem_t ** segment_semaphores = malloc(num_of_segments*sizeof(*segment_semaphores));
 
    for (int i = 0; i < num_of_segments; i++) {

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
    int *ready_children = malloc(num_of_segments*sizeof(int));  
    for(int i = 0;i < num_of_segments; i++){ 
        ready_children[i] = 0;  
    }   

    // 0 children have finished their requests
    sm->finished = 0;

    int *pid = malloc(Number_of_Childs*sizeof(int));
    for(int i = 0; i<Number_of_Childs ; i++) {
        
        pid[i] = fork();
        
        if(pid[i] < 0) {
            perror("Fork failed");
            exit(1);
        }

        // Children code
        if (pid[i] == 0) {
            child(number_of_requests , num_of_segments , i , Number_of_Childs , Segmentation_Degree , ready_children , sm, segment_semaphores , Parent_Ready_For_Request , Parent_Answer_Request , Child_Ready_For_Segment );
        }
    }

    // Until all children fineshed their requests
    while (sm->finished < Number_of_Childs) {
        
        // wait until child upload desired segment
        if(sem_wait(Child_Ready_For_Segment) < 0) {
            perror("sem wait Child_Ready_For_Segment semaphore at parent failed");
            exit(1);
        }

        // Finished parent programm
        if (sm->finished == Number_of_Childs) break;

        // sleep for 20ms
        usleep(20000);

        //  child upload this segment
        int desired_segment = sm->request_segment;

        // find this segment
        char line[MAX_LINE];

        // Find desired segment
        for(int i = 0; i < desired_segment; i ++) {
            for(int j = 0 ; j < Segmentation_Degree; j++) {
                fgets(line,MAX_LINE,fptr);
            }
        }
        // Upload desired segment to shared memory
        for(int j = 0 ; j < Segmentation_Degree; j++) {
                fgets(line,MAX_LINE,fptr);
                strcpy(sm->buffer[j],line);
        }
        fseek(fptr,0,SEEK_SET);


        if(sem_post(Parent_Answer_Request) < 0) {
            perror("sem_post parent answer at parent failed");
            exit(1);
        }
               
    }
    
    free(ready_children);
    free(pid);
    free(segment_semaphores);
    return (0);
}