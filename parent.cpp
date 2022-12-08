#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "shared_memory.h"


#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)


int main(int argc, char *argv[]){

    // Info from CommandLine
    char* filename;
    filename = argv[1];

    int Partition_Degree = atoi(argv[2]);   // Bathmos Katatmisis
    int Kids_Num = atoi(argv[3]);           // Number of how many kids
    int requests_count = atoi(argv[4]);     // Plithos aithsewn


    // Initialize 3 Semaphores
    sem_t* requestProducer = sem_open("requestProducer", O_CREAT | O_EXCL, SEM_PERMS, 0);

    if(requestProducer == SEM_FAILED){
        perror("sem_open From 'Request Producer' failed");
        exit(EXIT_FAILURE); // return -1
    }

    sem_t *answerProducer = sem_open("answerProducer", O_CREAT | O_EXCL, SEM_PERMS, 0);

    if (answerProducer == SEM_FAILED)
    {
        perror("sem_open From 'Answer Producer' failed");
        exit(EXIT_FAILURE);
    }

    sem_t *answerConsumer = sem_open("answerConsumer", O_CREAT | O_EXCL, SEM_PERMS, 0);

    if (answerConsumer == SEM_FAILED)
    {
        perror("sem_open From 'Answer Consumer' failed");
        exit(EXIT_FAILURE);
    }


    // arxeio se kommatia
        //(done)

    //ftianxo to shm.id:shered_memory id 
    // arxikopoio to semlck
sharedMemory semlock;
    // Create memory segment
    int Shared_Memory_id ;
    bool In_Use[Partition_Degree];
    
    int x;
    for (x;x<Partition_Degree;x++){
        In_Use[x] = false;
    }
    semlock->in_use = In_Use;
    
    
    
    if((Shared_Memory_id = shmget(IPC_PRIVATE, sizeof(sharedMemory), (S_IRUSR|S_IWUSR))) == -1){
        perror("Failed to create shared memory segment");
        return 1;
    }

    // Attach memory segment
    if((semlock = (sharedMemory)shmat(Shared_Memory_id, NULL, 0)) == (void*)-1){
        perror("Failed to attach memory segment");
        return 1;
    }

     for (i = 0; i < K; i++)
    {
        if ((pids[i] = fork()) < 0)
        { // Fork new process
            perror("Failed to create process");
            return 1;
        }
        if (pids[i] == 0){

        }
    }

    //ftianxo ta paidia 
        //ola ayta N fores
            //ta paidia prepei na zitane zonh kai grammi
            //ta paidia prepei na perimenoyn mexri h shm(SHARED memory) na exei thn zoni kai thn grammi poy zitane
            //na dinei mia apontisi
            //na perimenoyn ligo 20ms
            //na zitane me tyxaio tropo mia allh tixaia zoni 'h thn idia zoni jana me allh grammi 
        //telos N foron + metrame to xrono poso pire olo ayto
// prepei na kanei Με την παραλαβή της κάθε γραμμής η διεργασία αποτυπώνει σε αρχείο καταγραφής (1 ανά 
// διεργασία παιδί) το χρόνο υποβολής του αιτήματος, το χρόνο της απάντησης, το αίτημα στη 
// μορφή <x,y> όπως αναλύθηκε παραπάνω και την ίδια τη γραμμή.
// Η μητρική διεργασία που φέρει την ευθύνη για την αντικατάσταση των τμημάτων του αρχείου 
// στη μνήμη επίσης αναφέρει το χρόνο εισόδου και εξόδου ενός τμήματος από τη μνήμη

   





}