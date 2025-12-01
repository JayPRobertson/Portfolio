#define MAX_LINE_LEN 30

//define Train type
typedef struct Train{
    int num;
    bool priority;
    char* direction;
    float load_time;
    float cross_time;
    bool ready; 
} Train;

//prototypes
Train* new_train(int num, bool priority, char* direction, float load_time, float cross_time);
Train* make_loaded(Train* train);