#define MAX_SEGMENT 30000
#define MAX_LINE 300
#include <string.h>

using namespace std;


struct shared_memory{
    int temp_Segment;  // request segment
    string buffer;
    int finished;
};

typedef struct shared_memory* Shared_memory;
