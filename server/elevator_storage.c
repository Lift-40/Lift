#include "elevator_storage.h"
#include "configuration.h"
#include "../elev_algo/elevator.h"

Elevator elev_states[MAX_ELEVATORS];

void initStates(){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        elev_states[i] = NULL;
    }
}

void addState(Elevator elev_state){
    int exists = 0;
    char inputIP[32] = elev_state.ip;
    for(int i = 0; i < MAX_ELEVATORS; i++){
        char storedIP[32] = elev_states[i].ip;
        if(strncmp( inputIP, storedIP, strlen(storedIP) ) == 0){
	    printf("The elev_state %s already exists, updated it\n",elev_state);
	    exists = 1;
            elev_states[i] = elev_state;
	}
    }
    if (exists == 0){
    	for(int i = 0; i < MAX_ELEVATORS; i++){
            if(elev_states[i] == NULL){
                printf("Adding new elevator struct with IP %s to index %d\n",elev_state.ip,i);
                elev_states[i] = elev_state;
            }
        }
    }
}

Elevator getState(char * inputIP){
    int exists = 0;
    for(int i = 0; i < MAX_ELEVATORS; i++){
        char storedIP[32] = elev_states[i].ip;
        if(strncmp( inputIP, storedIP, strlen(storedIP) ) == 0){
	    printf("The elev_state %s exists, returned it\n",elev_state);
	    exists = 1;
            return elev_states[i];
	}
    }
    if (exists == 0){
	return NULL;
    }
}

// For now it's not needed
void removeState(Elevator elev_state){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(strncmp( inputIP, storedIP, strlen(storedIP) ) == 0){
            printf("Removing elevator struct with IP %s from index %d\n",elev_state.ip,i);
            elev_states[i] = NULL;
        }
    }
}

void storeElevator(Elevator elev) {
    addState(elev);
}

Elevator readElevator(char *ip) {
    return getState(ip);
}
