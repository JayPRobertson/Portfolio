#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "train.h"

Train* new_train(int num, bool priority, char* direction, float load_time, float cross_time){
    //create a train object with provided data
    
    Train* temp = (Train*)malloc(sizeof(Train));
    if (temp == NULL){
        printf("Error creating train %i", num);
        exit(EXIT_FAILURE);
    }
    
    temp->num = num;
    temp->priority = priority;
    temp->direction = strdup(direction);
    temp->load_time = load_time;
    temp->cross_time = cross_time;
    
    if (!load_time){ //if begins loaded
        temp->ready = true;
    }else{
        temp->ready = false;
    } 
    
    return temp;
}

Train* make_loaded(Train* train){
    //set the train to loaded
    train->ready = true;
    return train;
}

