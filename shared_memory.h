#define MAX_LINE 300

struct shared_memory{
    int temp_Segment;  // request segment
    char buffer[MAX_LINE];
    int finished;
};

typedef struct shared_memory* Shared_Memory;
