#define MAX_LINE 300
#define MAX_SR 300

struct shared_memory{
    int request_segment; 
    int given_segment;
    char buffer[MAX_SR][MAX_LINE];    
    int finished;
};


typedef struct shared_memory* SharedMemory;