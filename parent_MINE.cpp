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

using namespace std;

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define MAX_LINE 300

struct shared_memory{
    int a;  // request segment
    char buffer[MAX_LINE];
    int finished;
};



string* txt_to_string_segments(int segmentation_degree , char* File_Name)
{
    ////// Make The txt to string ////////////
    ifstream file(File_Name, ios::binary);
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

    string* array1 = new string[segmentation_degree];         // Array with the segmended string in its rows
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
        //cout << k;
    X = X + 1 ;     // lines counter to stop the while
    }

    //Print the Array
    // for (x=0; x< segmentation_degree ; x++){
    //     cout << array1[x] << '\n' << '\n' << '\n';
    // }

    return array1 ;
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

    
    // Create Txt File to Segments as Strings in an Array
    string* Segments_String = txt_to_string_segments(Segmentation_Degree , File_Name);






    // // Check if file is small
    // if (lines < 1000) {
    //     perror("You gave me a small text file\n");
    //     exit(1);
    // }
    
    // // Check if segmentation rate is bigger more than lines
    // if (Segmentation_Rate > lines) {
    //     perror("Lines are bigger more than segmentation rate\n");
    //     exit(1);
    // }
    
    // // Make an array of pointers to every line of your file
    // // FILE **array_of_lines = malloc(lines*sizeof(FILE*));
    // // array_of_lines[0] = fptr;
    // // int l = 1;
    // // for (int i = getc(fptr); i != EOF; i = getc(fptr)) {    
    // //     if (i == '\n') {
    // //         getc(fptr);
    // //         array_of_lines[l] = fptr;
    // //         l++;
    // //     }
    // // }
    // // fseek(fptr,0,SEEK_SET);

    // // xwrismos se segments
    // FILE **array_of_segments = malloc(Segmentation_Rate*lines*sizeof(FILE*));
    // array_of_segments[0] = file_ptr;
    // for (int i = getc(file_ptr); i != EOF; i = getc(file_ptr)) {    
    //     int k = 0;
    //     if (i == '\n') {
    //         getc(file_ptr);
    //         // array_of_lines[l] = fptr;
    //         k++;
    //     }
    // }
    // fseek(file_ptr,0,SEEK_SET);



    // // Create shared memory
    // struct shared_memory* Shared_memory;
    // int shmid;
    // if((shmid = shmget(IPC_PRIVATE, sizeof(Shared_memory), (S_IRUSR|S_IWUSR))) == -1){
    //     perror("Failed to create shared memory");
    //     exit(1);
    // }

    // // Attach memory segment
    // if((Shared_memory = shmat(shmid, NULL, 0)) == (void*)-1){
    //     perror("Failed to attach memory segment");
    //     exit(1);
    // }
    
    // // Create request semaphore

    // sem_t *generic = sem_open("generic", O_CREAT, SEM_PERMS, 0);
    // if (generic == SEM_FAILED) {
    //     perror("request parent semaphore error");
    //     exit(1);
    // }

    // // Child is ready to post new segment on shared memory

    // sem_t *request_parent = sem_open("request_parent", O_CREAT, SEM_PERMS, 0);
    // if (request_parent == SEM_FAILED) {
    //     perror("request parent semaphore error");
    //     exit(1);
    // }

    // // Parent upload text segment to shared memory

    // sem_t *parent_answer = sem_open("parent_answer", O_CREAT, SEM_PERMS, 0);
    // if (parent_answer == SEM_FAILED) {
    //     perror("request parent semaphore error");
    //     exit(1);
    // }

    // // All children ready

    // sem_t *children_ready = sem_open("children_ready", O_CREAT, SEM_PERMS, 0);
    // if (children_ready == SEM_FAILED) {
    //     perror("request parent semaphore error");
    //     exit(1);
    // }


    // // Create an array of semaphoreshes one per segment
    // sem_t ** segment_semaphores = malloc(num_of_segments*sizeof(*segment_semaphores));
 
    // for (int i = 0; i < num_of_segments; i++) {

    //     char buffer[120];
    //     snprintf(buffer, sizeof(buffer), "%s%d", "segment", i);
        
    //     segment_semaphores[i] = sem_open(buffer, O_CREAT, SEM_PERMS, 0);
    //     if (segment_semaphores[i] == SEM_FAILED) {
    //         fprintf(stderr, "%dth semaphore open fail\n", i);
    //         exit(1);
    //     }
    // }

    

    // // Create childs
    // int *pid = malloc(N*sizeof(int));
    // for(int i = 0; i<N ; i++) {
    //    pid[i] = fork();
    // }

    // srand(time(NULL));
    // for(int i = 0; i < N; i++) {
    //     if(pid[i] < 0) {
    //         perror("Fork failed");
    //         exit(1);
    //     }

    //     // Children code
    //     if (pid[i] == 0) {
    //         // Number of requests
    //         int k = 0;
    //         while (k<number_of_requests) {

    //             // <a,b> = <segment,line>
    //             int a,b;
    //             if (k == 0) {
    //                 a = rand()%num_of_segments;
    //                 b = rand()%(lines/s_r); 
    //             }
    //             else {
    //                ;
    //             }
    //             if(sem_wait(generic) < 0) {  
    //             fprintf(stderr, "%dth semaphore wait fail\n", i);
    //             exit(1);
    //             }

    //             Shared_memory->a = a;
                
    //             if(sem_post(request_parent < 0)) { 
    //                 perror("request_parent_post failed on parent");
    //                 exit(1);
    //             }

    //             if(sem_wait(parent_answer) < 0) {  
    //             fprintf(stderr, "%dth semaphore wait fail\n", i);
    //             exit(1);
    //             }

    //         }           
    //         return;
    //     } 
    //     if (i==N-1) {    // an einai to teleutaio paidi poy dimioyrgeitai
    //         if(sem_post(children_ready < 0)) { 
    //             perror("request_parent_post failed on parent");
    //             exit(1);
    //         }
    //     }
    // }

    // // Parent code

    // if(sem_wait(children_ready) < 0) {  
    //     perror("Wait children_ready error\n");
    //     exit(1);
    // }

    // int ready_children = 0;         // pote teleiwsan oles tis aithseis toys ta paidia
    // while (ready_children < N) {

    //    if(sem_post(generic < 0)) { 
    //         perror("generic post failed on parent");
    //         exit(1);
    //     }

    //     //  anevazo to segment stin kyria mnimi
    //     int a_from_child = Shared_memory->a; // pairno to segment
    //     fgets(Shared_memory->buffer,MAX_LINE,array_of_lines[s_r*a_from_child]);  
    
    //     // get line from children
    //     // Shared_memory->finished = 0;
    //     // int a_from_child = Shared_memory->a;
    //     // int b_from_child = Shared_memory->b;
    //     // int num_of_line = a_from_child*s_r + b_from_child;
    //     // // give line to children
    //     // fgets(Shared_memory->buffer,MAX_LINE,array_of_lines[num_of_line]);

    // }
}