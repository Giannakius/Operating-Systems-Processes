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
#include "shared_memory.h"

using namespace std;

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)


string* txt_to_string_segments(int Segmentation_Degree , string fileStr , int string_size , int lines_per_section)
{

    string* array1 = new string[Segmentation_Degree];         // Array with the segmended string in its rows
    int temp = 0 , k = 0 , start = 0 , counter =0 , X = 0 ;

    while (X<Segmentation_Degree){
        while (k<=string_size and counter<lines_per_section){
            k++;    // find all the chars until the new line
            if (fileStr[k]=='\n'){
                counter++;
            }
        }
        array1[temp] = fileStr.substr(start,k);     // copy them to the array [temp]
        temp = temp + 1;
        start = start + k +1;   // fix the start for the new start
        //cout << k;
    X = X + 1 ;     // lines counter to stop the while
    }

    //Print the Array
    // for (x=0; x< Segmentation_Degree ; x++){
    //     cout << array1[x] << '\n' << '\n' << '\n';
    // }

    return array1 ;
}


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





int main(int argc, char *argv[]) {
    
    // Check arguements to be 5
    if (argc != 5) {
        perror("I want 5 arguements :P \n");
        exit(1);
    }
    // Open file
    FILE* file_ptr;
    file_ptr = fopen(argv[1],"r");
    if (file_ptr == NULL) {
        perror("You gave me an empty file :P \n");
        exit(1);
    }

    char* File_Name = argv[1];

    int Segmentation_Degree = atoi(argv[2]);  // Bathmos Katatmisis
    int Number_Of_Childs = atoi(argv[3]);
    int Number_of_requests = atoi(argv[4]); //  from every child


    ////// Make The txt to string ////////////
    ifstream file(File_Name, ios::binary);
    string fileStr;

    istreambuf_iterator<char> inputIt(file), emptyInputIt;
    back_insert_iterator<string> stringInsert(fileStr);

    copy(inputIt, emptyInputIt, stringInsert);
    //////////////////////////////////////////

    int String_Size = fileStr.length();     // Length of .txt file
    
    int Lines_Count = 0 , x ;
    for (x=0;x<String_Size;x++){
        if (fileStr[x]=='\n'){
            Lines_Count++;  // athroisma grammwn sto text
        }
    }

    int Lines_Per_Segment = (Lines_Count+1) / Segmentation_Degree ;   // grammes ana tmima 

    int Num_of_Segments ; // Number of Segments

    if (Lines_Count % Segmentation_Degree == 0){
        Num_of_Segments = Lines_Count % Segmentation_Degree ;
    }
    else {
        Num_of_Segments = Lines_Count/Segmentation_Degree + 1;
    }

    // Create Txt File to Segments as Strings in an Array
    string* Segments_String = txt_to_string_segments(Segmentation_Degree , fileStr , String_Size ,Lines_Per_Segment);
    
    for (x=0; x< Segmentation_Degree ; x++){
        cout << Segments_String[x] << '\n' << '\n' << '\n';
    }


///////////////////////////////////////////////////////////////////////////////////////////////////////////
                // WORKING TILL HERE//
//////////////////////////////////////////////////////////////////////////////////////////////////////


    // Create shared memory
    Shared_memory s_m;
    int shmid;

    if((shmid = shmget(IPC_PRIVATE, sizeof(Shared_memory), (S_IRUSR|S_IWUSR))) == -1){
        perror("Failed to create shared memory");
        exit(1);
    }

    // Attach memory segment
    Shared_memory = (Shared_Memory*)shmat(shmid, NULL, 0);
    if(Shared_memory == (void*)-1){
        perror("Failed to attach memory segment");
        exit(1);
    }
    



    // Child is ready to post new segment on shared memory

    sem_t *request_parent = sem_open("request_parent", O_CREAT, SEM_PERMS, 0);
    if (request_parent == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }

    // Parent upload text segment to shared memory

    sem_t *answerConsumer = sem_open("answerConsumer", O_CREAT, SEM_PERMS, 0);
    if (answerConsumer == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }

    // All children ready

    sem_t *answerProducer = sem_open("answerProducer", O_CREAT, SEM_PERMS, 0);
    if (answerProducer == SEM_FAILED) {
        perror("request parent semaphore error");
        exit(1);
    }


    // Create an array of semaphoreshes one per segment
    sem_t** segment_semaphores;
 
    for (int i = 0; i < Segmentation_Degree; i++) {
        segment_semaphores[i] = new sem_t[Num_of_Segments] ;

        char buffer[120];
        snprintf(buffer, sizeof(buffer), "%s%d", "segment", i);
        // Arxikopoiw oloys shmaioforous se 1
        segment_semaphores[i] = sem_open(buffer, O_CREAT, SEM_PERMS, 1);
        if (segment_semaphores[i] == SEM_FAILED) {
            fprintf(stderr, "%dth semaphore open fail\n", i);
            exit(1);
        }
    }

    

    // Create childs

    pid_t  pid[Number_Of_Childs];

    srand(time(NULL));

    int read_count[Segmentation_Degree];
    int a;
    for (a=0;a<Segmentation_Degree;a++){
        read_count[a] = 0;
    }

    for(int i = 0; i<Number_Of_Childs ; i++) {
       pid[i] = fork();
    
        if(pid[i] < 0) {
        perror("Fork failed");
        exit(1);
        }

        // Children code

        if (pid[i] == 0) {
            
            // Number of requests
            int k = 0;

            while (k<Number_of_requests) {
                int curr_segment , curr_line;
                if (k == 0) {
                    curr_segment = rand() % Num_of_Segments;
                } else {
                    curr_segment = Rand_Segment(curr_segment , Num_of_Segments) ;
                }
                curr_line = rand() % Lines_Per_Segment;
                // Ean eimaste sto teleytaio tmhma mporei na min yparxoyn akribws Lines_Per_Segment grammes
                if (k==Segmentation_Degree-1){
                    curr_line = 0;///////////////////////dwahgfdkashjgehfgeashjkfgshejkfghjsegfahjesgfhjse
                }///////////////////////agkesjhgfjhksgfhsjegfhsejgfkajhesfghjgfsehjgfshjgfhsjefgashej
                //easulhfgshljkafjhesfjhsejfklsehjfhasejkfl
                
            
                if(sem_wait(segment_semaphores[curr_segment]) < 0) {
                    perror("Wait error");
                    exit(1);
                }

                read_count[curr_segment]++;

                if(read_count[curr_segment] == 1) {

                    if(sem_wait(request_parent) < 0) {
                        perror("Wait error");
                        exit(1);
                    }

                    s_m->temp_Segment = curr_segment;
                    
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
                }

                if(curr_segment == s_m->temp_Segment) {
                    char* temp_buffer_text[MAX_LINE];
                    strcpy(temp_buffer_text , s_m->buffer[curr_line]);
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
                return ;
            } 
        }
    }

    // // Parent code

    while(s_m->finished < Number_Of_Childs) {
        
        if(sem_wait(answerConsumer) < 0) {
            perror("sem wait to parent failed");
            exit(1);
        }

        s_m->buffer = Segments_String[s_m->temp_Segment] ;

        if(sem_post(answerProducer)) {
            perror("sem post to parent failed");
            exit(1);
        }
    }
}