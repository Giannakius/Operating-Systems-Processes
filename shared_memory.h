#define MAX_SEGMENT 30000
#defint MAX_LINE 300
struct shared_memory{
    int temp_Segment;  // request segment
    char buffer[MAX_SEGMENT];
    int finished;
};

typedef struct shared_memory* Shared_memory;
