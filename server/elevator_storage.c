#include "elevator_storage.h"
#include "../configuration.h"
#include "../elevator/elevator.h"
#include "../elevator/elevator_io_types.h"
#include "../drivers/network_io.h"
#include <string.h>
#include <stdio.h>

Elevator elev_states[MAX_ELEVATORS];

void initStates(){
    for(int i = 0; i < MAX_ELEVATORS; i++){
		Elevator emptyElevator;
		emptyElevator.isEmpty = true;
		emptyElevator.ip[0] = 0;
		emptyElevator.elevatorID = 0;
		memcpy(&elev_states[i], &emptyElevator, sizeof(Elevator));
    }
}

void addState(Elevator elev_state){
    int exists = 0;
	int stored = 0;
	//printf("Attempting to add new elevator struct with IP %s\n",elev_state.ip);
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(elev_states[i].elevatorID == elev_state.elevatorID){
			printf("(elevator_storage.c)The elev_state with ID %d already exists in position %d, updated it\n",elev_state.elevatorID,i);
			exists = 1;
			memcpy(&elev_states[i], &elev_state, sizeof(Elevator));
		}
    }
    if (exists == 0){
    	for(int i = 0; i < MAX_ELEVATORS; i++){
            if(elev_states[i].isEmpty == true && stored == 0){
                printf("(elevator_storage.c)Adding new elevator struct with ID %d to index %d\n",elev_state.elevatorID,i);
                memcpy(&elev_states[i], &elev_state, sizeof(Elevator));
				stored = 1;
            }
        }
    }
}

Elevator getState(int elevatorID){
    int exists = 0;
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(elev_states[i].elevatorID == elevatorID){
	    	printf("(elevator_storage.c)The elev_state with ID %d exists in position %d, returned it\n",elev_states[i].elevatorID, i);
	    	exists = 1;
            return elev_states[i];
		}
    }
    if (exists == 0){
		Elevator emptyElevator;
		emptyElevator.isEmpty = true;
		emptyElevator.elevatorID = 0;
		printf("(elevator_storage.c)The elev_state with ID %d does not exist, returned empty elevator \n", elevatorID);
		return emptyElevator;
    }
}

// For now it's not needed
void removeState(int elevatorID){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(elev_states[i].elevatorID == elevatorID){
            printf("(elevator_storage.c)Removing elevator struct with ID %d from index %d\n",elevatorID,i);
			Elevator emptyElevator;
			emptyElevator.isEmpty = true;
			emptyElevator.elevatorID = 0;
            elev_states[i] = emptyElevator;
        }
    }
}

void storeElevator(Elevator elev) {
    addState(elev);
}

Elevator readElevator(int elevatorID) {
	printf("(elevator_storage.c)Attempting to read elevator %d\n", elevatorID);
    return getState(elevatorID);
}

Elevator readElevatorByIndex(int index) {
    return elev_states[index];
}
