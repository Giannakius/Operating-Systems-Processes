#define MAX_LINE 300
#define MAX_SEGMENTATION_DEGREE 300


struct shared_memory{
    int request_segment; 
    int given_segment;
    char buffer[MAX_SEGMENTATION_DEGREE][MAX_LINE];    
    int finished;
};


typedef struct shared_memory* SharedMemory;

void child (int ,int ,int ,int , int  , int*  , SharedMemory , sem_t** ,void* ,void* ,void*);