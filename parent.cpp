#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "shared_memory.h"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)


int text_separation(const char * txt_name , int segmentation_degree)
{
    ////// Make The txt to string ////////////
    ifstream file(txt_name, ios::binary);
    string fileStr;

    istreambuf_iterator<char> inputIt(file), emptyInputIt;
    back_insert_iterator<string> stringInsert(fileStr);

    copy(inputIt, emptyInputIt, stringInsert);
    //////////////////////////////////////////

    int string_size = fileStr.length();     // Length of .txt file
    
    int lines_count = 0 ;                   // Count how many rows in .txt
    int x;
    lines_count = 0 ;
    for (x=0;x<string_size;x++){
        if (fileStr[x]=='\n'){
            lines_count++;  // athroisma grammwn sto text
        }
    }

    int lines_per_section = (lines_count+1) / segmentation_degree ;   // grammes ana tmima 
    
    string array1[segmentation_degree];         // Array with the segmended string in its rows
    int temp = 0 , k = 0 , start = 0 , counter =0 , X = 0 ;

    while (X<segmentation_degree){
        while (k<=string_size and counter<lines_per_section){
            k++;    // find all the chars until the new line
            if (fileStr[k]=='\n'){
                counter++;
            }
        }
        array1[temp] = fileStr.substr(start,k);     // copy them to the array [temp]
        temp = temp + 1;
        start = start + k +1;   // fix the start for the new start
    X = X + 1 ;     // lines counter to stop the while
    }

    return array1;
}


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

    // Children array
    pid_t pids[Kids_Num];
    
    int i ;
    for (i = 0; i < Kids_Num; i++)
    {
        if ((pids[i] = fork()) < 0)
        { // Fork new process
            perror("Failed to create process");
            return 1;
        }
        if(pids[i] == 0){          // If it is child process
            child(File_Lines, Partition_Degree, i, semlock, requestProducer, answerProducer, answerConsumer);
            exit(0);
        }
        if (== 1){

        }
    }

    // epilogh tyxaia 
    srand((unsigned) time(NULL));
    int random_line = rand() % Partition_Degree ;
    





}