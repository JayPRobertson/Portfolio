#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "train.h"
#include <time.h>

#define MAX_LINE_LEN 30

//global variables
float system_time = 0.0; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char* last_direction = "East";
bool is_starved = false;
Train* readyQ;
int ready_size = 0;

//prototypes
int open_file(char* filename, Train** trains);
int get_trains(char* line, Train** trains, int i);
void* run_train(void* arg);
bool check_train(Train* train);
void format_time(char* buffer, float time);

int main(int argc, char* argv[]){
    //check if too many arguments
    if (argc !=2){ 
        perror("Too many arguments");
    }
    
    Train* trains;
    char* filename = argv[1];
    pthread_t* threads;
    int num_trains;

    num_trains = open_file(filename, &trains);
    threads = malloc((num_trains+1) * sizeof(pthread_t));
    
    for (int i = 0; i < num_trains; i++){
        if (pthread_create(&threads[i], NULL, run_train, &trains[i]) != 0) {
            perror("Failed to create thread");
            free(trains);
            free(threads);
            pthread_mutex_destroy(&mutex);
            exit(EXIT_FAILURE);
        }
    }
    
    //wait for threads to finish
    for (int i = 0; i < num_trains; i++){
        pthread_join(threads[i], NULL);
    }  
    
    free(threads);
    free(trains);
    free(readyQ);
    pthread_mutex_destroy(&mutex);
    
    return EXIT_SUCCESS;
}

int open_file(char* filename, Train** trains){
    //opens the given file and stores the data in trains
    
    char line[MAX_LINE_LEN];
    int cur = 0;
    FILE* file = fopen(filename, "r");
    int num_trains = 0;
    
    //check file isn't empty
    if (file == NULL){
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, MAX_LINE_LEN, file)){
        num_trains++; //get number of lines
    }
    
    //allocate memory for trains
    *trains = malloc(num_trains * sizeof(Train));
    readyQ = malloc(num_trains * sizeof(Train));
    
    if (*trains == NULL) {
        perror("Allocation for trains failed");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    
    rewind(file);
    
    //get each train info and store in trains
    while (fgets(line, MAX_LINE_LEN, file) != NULL){
        if (get_trains(line, trains, cur)==-1){
            perror("Direction not found");
            free(trains);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        cur++;
    }
    
    fclose(file);
    return num_trains;
}

int get_trains(char* line, Train** trains, int i){
    //creates a train object from a line and stores it in trains
    char token;
    bool priority; //false is low, true is high
    char* direction;
    float load_time;
    float cross_time;
    
    token = strtok(line, " ")[0];
        switch(token){
            case 'e':
                priority = false;
                direction = "East";
                break;
            case 'w':
                priority = false;
                direction = "West";
                break;
            case 'E':
                priority = true;
                direction = "East";
                break;
            case 'W':
                priority = true;
                direction = "West";
                break;
            default:
                return -1;
        }
        load_time = atof(strtok(NULL, " "))/10;
        cross_time = atof(strtok(NULL, " "))/10;
        
        //printf("Train%d: D = %s, L=%f, C=%f, P=%d\n", i, direction, load_time, cross_time, priority);
        
        //store in array of trains
        (*trains)[i] = *new_train(i, priority, direction, load_time, cross_time);
        
        return 1;
}

void* run_train(void* arg){
    Train* train = (Train*)arg;
    char timestr[20];
    float cur_time;
    
    //load train and put in readyQ
    usleep(train->load_time * 100000); 
    if (system_time < train->load_time) {
        system_time = train->load_time;
    }
    format_time(timestr, system_time);
    printf("%s Train %2d is ready to go %s\n", timestr, train->num, train->direction);  
    make_loaded(train);
    readyQ[ready_size] = *train;
    ready_size++;

    //Check if it should be the next train to go on the main track
    while (true){
        if (!pthread_mutex_trylock(&mutex)){
            if (check_train(train)){
                break;
            } else{
                pthread_mutex_unlock(&mutex);
            }
        }
    }
    
    //run train crossing
    format_time(timestr, system_time);
    cur_time = system_time;
    printf("%s Train %2d is ON the main track going %s\n", timestr, train->num, train->direction);  
    usleep(train->cross_time * 100000);  //simulate crossing time

    //Check if the system will be starved
    if ((system_time - train->load_time)){
        if (!strcmp(train->direction, last_direction) && ready_size > 1){
            is_starved = true;
        }else{
            is_starved = false;
        }
    }   
    
    //take train off of track
    last_direction = train->direction;
    system_time = (cur_time+train->cross_time);
    format_time(timestr, system_time);
    printf("%s Train %2d is OFF the main track after going %s\n", timestr, train->num, train->direction);
    
    //find index of train (may not be first item in readyQ)
    int index;
    for (int i = 0; i < ready_size; i++){
        if (readyQ[i].num == train->num){
            index = i;
            break; 
        }
    }
    
    //remove item from readyQ and kill thread
    for (int k = index; k < ready_size-1; k++){
        readyQ[k] = readyQ[k + 1];
    }
    ready_size--;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

bool check_train(Train* train) {
    //checks whether passed train should get the mutex
    Train* cur_train;

    if (ready_size == 1) { //if only one train ready
        return true;
    }

    if (is_starved) { //if system starved
        if (!strcmp(train->direction, last_direction)) {
            for (int i = 0; i < ready_size; i++) {
                cur_train = &readyQ[i];

                if (cur_train->num != train->num) {
                    if (strcmp(cur_train->direction, last_direction)) {
                        return false;
                    }
                }
            }
            //return strcmp(train->direction, last_direction) == 0;
        }
    }

    // Normal check for train priority
    for (int i = 0; i < ready_size; i++){
        cur_train = &readyQ[i];

        if (cur_train->num != train->num){
            
            // if exists loaded train with higher priority, false
            if (!train->priority && cur_train->priority) {
                if (!is_starved) {
                    return false;
                }
                if (is_starved && !strcmp(train->direction, last_direction)){
                    return false;
                }
            }

            //if trains have the same high priority
            if (train->priority && cur_train->priority){
                
                //if same direction
                if (!strcmp(train->direction, cur_train->direction)){
                    
                    //if same loading time, first in input file
                    if (train->load_time == cur_train->load_time){
                        return train->num < cur_train->num;
                    }
                    return train->load_time < cur_train->load_time;
                }
                return strcmp(train->direction, last_direction) == 0;
            }
        }
    }

    return true;
}


void format_time(char* timestr, float time){
    //nicely formats the time into a provided string (accurate to +/-0.1)
    int hours = (int)(time / 3600);
    int minutes = (int)((time - hours * 3600) / 60);
    int seconds = (int)(time - hours * 3600 - minutes * 60);
    int tenths = (int)((time - (int)time) * 10);
    
    snprintf(timestr, 20, "%02d:%02d:%02d.%1d", hours, minutes, seconds, tenths);
}