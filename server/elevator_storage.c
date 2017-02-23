#include "elevator_storage.h"
#include "../configuration.h"
#include "../elev_algo/elevator.h"
#include "../elev_algo/elevator_io_types.h"
#include "../network_driver/network_io.h"
#include <string.h>
#include <stdio.h>

Elevator elev_states[MAX_ELEVATORS];

void initStates(){
    for(int i = 0; i < MAX_ELEVATORS - 1; i++){
		Elevator emptyElevator;
		emptyElevator.isEmpty = true;
        elev_states[i] = emptyElevator;
    }
}

void addState(Elevator elev_state){
    int exists = 0;
	int stored = 0;
    char inputIP[32] = "";
	printf("Attempting to add new elevator struct with IP %s\n",elev_state.ip);
	strcpy(inputIP, elev_state.ip);
    for(int i = 0; i < MAX_ELEVATORS - 1; i++){
		printf("%d\n",i);
		char storedIP[32] = "";
		strcpy(storedIP, elev_states[i].ip);
        //char storedIP[32] = elev_states[i].ip;
        if(strcmp( inputIP, storedIP ) == 0){
			printf("The elev_state %s already exists, updated it\n",elev_state.ip);
			exists = 1;
			elev_states[i] = elev_state;
		}
    }
    if (exists == 0){
    	for(int i = 0; i < MAX_ELEVATORS - 1; i++){
            if(elev_states[i].isEmpty && stored == 0){
                printf("Adding new elevator struct with IP %s to index %d\n",elev_state.ip,i);
                elev_states[i] = elev_state;
				stored = 1;
            }
        }
    }
}

Elevator getState(char * inputIP){
    int exists = 0;
    for(int i = 0; i < MAX_ELEVATORS - 1; i++){
		char storedIP[32] = "";
		strcpy(storedIP, elev_states[i].ip);
        if(strcmp( inputIP, storedIP) == 0){
	    	printf("The elev_state %s exists, returned it\n",elev_states[i].ip);
	    	exists = 1;
            return elev_states[i];
		}
    }
    if (exists == 0){
		Elevator emptyElevator;
		emptyElevator.isEmpty = true;
		return emptyElevator;
    }
}

// For now it's not needed
void removeState(Elevator elev_state){
    char inputIP[32] = "";
	strcpy(inputIP, elev_state.ip);
    for(int i = 0; i < MAX_ELEVATORS - 1; i++){
		char storedIP[32] = "";
		strcpy(storedIP, elev_states[i].ip);
        if(strcmp( inputIP, storedIP ) == 0){
            printf("Removing elevator struct with IP %s from index %d\n",elev_state.ip,i);
			Elevator emptyElevator;
            elev_states[i] = emptyElevator;
        }
    }
}

void storeElevator(Elevator elev) {
    addState(elev);
}

Elevator readElevator(char *ip) {
    return getState(ip);
}
