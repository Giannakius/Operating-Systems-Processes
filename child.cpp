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


void child (int Number_of_requests,int curr_segment,int Num_of_Segments,int curr_line,int Lines_Per_Segment, int Segmentation_Degree , int* read_count , Shared_memory s_m, sem_t** segment_semaphores,sem_t* request_parent,sem_t* answerConsumer,sem_t* answerProducer ){
    
    // Number of requests
    int k = 0;
    while (k<Number_of_requests) {
        if (k == 0) {             
            srand(time(NULL) ^ (getpid()<<16));
            curr_segment = rand() % Num_of_Segments;
            cout << "first segment with random segment :"<< curr_segment << endl;

        } else {
            curr_segment = Rand_Segment(curr_segment , Num_of_Segments) ;
            cout << k << " segment with random segment :"<< curr_segment << endl;
        }
        curr_line = rand() % Lines_Per_Segment;
        // Ean eimaste sto teleytaio tmhma mporei na min yparxoyn akribws Lines_Per_Segment grammes
        if (k==Segmentation_Degree-1){
            curr_line = 0;///////////////////////dwahgfdkashjgehfgeashjkfgshejkfghjsegfahjesgfhjse
        }///////////////////////agkesjhgfjhksgfhsjegfhsejgfkajhesfghjgfsehjgfshjgfhsjefgashej
        //easulhfgshljkafjhesfjhsejfklsehjfhasejkfl

        cout << "firstttttttttttt " << curr_segment << endl;
        int value;
        sem_getvalue(segment_semaphores[curr_segment],&value);
        cout <<"value = " <<  value << endl;
        if(sem_wait(segment_semaphores[curr_segment]) < 0) {
            perror("Wait error");
            exit(1);
        }
        cout << "second " << curr_segment << endl; 

        read_count[curr_segment]++;
        cout <<"read count [" << curr_segment << "] = " <<  read_count[curr_segment] << endl;                
        if(read_count[curr_segment] == 1) {

            if(sem_wait(request_parent) < 0) {
                perror("Wait error");
                exit(1);
            }

            s_m->temp_Segment = curr_segment;
            cout << "AEKARA = " << s_m->temp_Segment << endl;

            if(sem_post(answerConsumer) < 0){
                perror("sem_post failed on child");
                exit(1);
            }

            // Wait for answer from producer
            if(sem_wait(answerProducer) < 0){
                perror("sem_wait failed on child");
                exit(1);
            }
        }
        
        if(sem_post(segment_semaphores[curr_segment]) < 0)  {
            perror("sem_wait failed on child");
            exit(1);
        }

        if(curr_segment == s_m->temp_Segment) {
            //char temp_buffer_text[MAX_LINE];
            //strcpy(temp_buffer_text , s_m->buffer[curr_line]);
            string buffer_text = new char[MAX_LINE];
            buffer_text = s_m->buffer[curr_line];
            k++;
        }

        if(sem_wait(segment_semaphores[curr_segment]) < 0){
            perror("sem_wait failed on child");
            exit(1);
        }

        read_count[curr_segment]--;

        if (read_count[curr_segment] == 0) {
            if(sem_post(request_parent) < 0) {
                perror("post error");
                exit(1);
            }
        }

        if(sem_post(segment_semaphores[curr_segment]) < 0)  {
            perror("sem_wait failed on child");
            exit(1);
        }
    }    
    s_m->finished++;       
}