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
#include <stdio.h>

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define MAX_LINE 300
#define MAX_SR 300

struct shared_memory{
    int request_segment; 
    int given_segment;
    char buffer[MAX_SR][MAX_LINE];    
    int finished;
};

typedef struct shared_memory* SharedMemory;

int main(int argc, char *argv[]) {
    
    // Check arguements
    if (argc != 5) {
        perror("Give me a wrong inputs\n");
        exit(1);
    }

    // Open file
    FILE* fptr;
    fptr = fopen(argv[1],"r");
    if (fptr == NULL) {
        perror("You gave me wrong file\n");
        exit(1);
    }
    
    // Get number of lines
    int lines = 0;
    for (char i = getc(fptr); i != EOF; i = getc(fptr)) {
        if (i == '\n') lines++;
    }
    lines++;
    fseek(fptr,0,SEEK_SET);
    
    // Segmentation rate
    int s_r = atoi(argv[2]);

    // Check if segmentation rate is big
    if (s_r > MAX_SR) {
        perror("Segmentation rate is bigger than more Max segmentation rate\n");
        exit(1);
    }

    // Number of segments
    int num_of_segments = lines%s_r ? lines/s_r + 1 : lines/s_r;
    

    // Check if file is small
    if (lines < 1000) {
        perror("You gave me a small text file\n");
        exit(1);
    }
    
    // Check if segmentation rate is bigger more than lines
    if (s_r > lines) {
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

    // Number of requests from every child
    int number_of_requests = atoi(argv[4]);

 
    // Create childs
    int N = atoi(argv[3]);

    
    // How many children is ready to read per segment   
    int *ready_children = malloc(num_of_segments*sizeof(int));  
    for(int i = 0;i < num_of_segments; i++){ 
        ready_children[i] = 0;  
    }   

    // 0 children have finished their requests
    sm->finished = 0;

    int *pid = malloc(N*sizeof(int));
    for(int i = 0; i<N ; i++) {
        
        pid[i] = fork();
        
        if(pid[i] < 0) {
            perror("Fork failed");
            exit(1);
        }
        clock_t answer_start , answer_end;
        answer_start = clock();
        
        // Children code
        if (pid[i] == 0) {

            char buffer[120];
            snprintf(buffer, sizeof(buffer), "%s%d", "child", i);
            FILE *f = fopen(buffer, "w");

            // Number of requests
            int k = 0;
            // clock for request and for answer
            
            while (k<number_of_requests) {
                
                int a,b; // a = desired segment , b = desired line
                if (k == 0) {
                    srand(time(NULL) ^ (getpid()<<16));
                    a = rand()%num_of_segments;
                    
                } else {
                    int p = rand()%10 + 1; // p = propability
                    if (p > 7) a = rand()%num_of_segments;
                }
                b = rand()%s_r;
               // clock_t request;
                //clock_t answer_start , answer_end;
                // clock starting when child find her line                
              //  request = clock();

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
                    sm->request_segment = a;

                    // request ends
                    //request = clock() - request;

                    // clock starting when child make request
                    //answer_start = clock();

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
                if(a == sm->request_segment) {

                    // child reading requesting line
                    char reading_line[MAX_LINE];
                    strcpy(reading_line,sm->buffer[b]);
                    
                    // child finished one request
                    k++;
                      usleep(200000);
                    // child get the answer
                    //  answer_end = clock();
                    // double time_taken = ((double)answer_end-answer_start)/CLOCKS_PER_SEC; // calculate the total elapsed time
                    //printf("Child %d waited on average %.15lf seconds for server response\n", getpid(), time_taken/N);
                    //printf("%.15f <%d,%d>\n %s \n",time_taken,a,b,reading_line);
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
            sm->finished++;
            
            // Conclusion to finished the parent programm
            if(sm->finished == N) {
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
        answer_end = clock();
        double time_taken = ((double)answer_end-answer_start)/CLOCKS_PER_SEC; // calculate the total elapsed time
        printf("peos %f \n",time_taken);
    }

    // Parent code

    // Until all children fineshed their requests
    while (sm->finished < N) {
        
        // wait until child upload desired segment
        if(sem_wait(child_ready) < 0) {
            perror("sem wait child_ready semaphore at parent failed");
            exit(1);
        }

        // Finished parent programm
        if (sm->finished == N) break;

        // sleep for 20ms
        usleep(20000);

        //  child upload this segment
        int desired_segment = sm->request_segment;

        // find this segment
        char line[MAX_LINE];

        // Find desired segment
        for(int i = 0; i < desired_segment; i ++) {
            for(int j = 0 ; j < s_r; j++) {
                fgets(line,MAX_LINE,fptr);
            }
        }
        // Upload desired segment to shared memory
        for(int j = 0 ; j < s_r; j++) {
                fgets(line,MAX_LINE,fptr);
                strcpy(sm->buffer[j],line);
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