
struct shared_memory{
    char *buffer;   // Content of the desired line
    bool *in_use;  // Child completed or not (0 for not completed, 1 for completed)
};

typedef struct shared_memory* sharedMemory;



