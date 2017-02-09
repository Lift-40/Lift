#include "elevator_manager.h"
#include "elevator_io_types.h"
#include "network_io.h"
#include "queue.h"
#include "configuration.h"

#include <stdio.h>
#include <stdlib.h>

char available_elevators[MAX_ELEVATORS][32];

int server_init() {
    networkInit();
}

int server_routine() {
    /* code goes here */
    Message msg;
    // Call receive message
    msg = receiveMessage();
    if (msg != NULL) {
	// if it's a request type then add it to the queue         
	if (msg.type == req){
	    storeRequest(msg.request);
	}
	// else if it's a state type then add it to the storage module
	else if (msg.type == elev_state) {
            storeElevator(msg.elev_struct);
	    // TODO:if state type, check if current first order of the queue is in the elevator's queue 
	    // if it is, remove order from server queue
        }
	// There's no need to handle the light type since those orders will only be sent to the elevators
    }

    Request firstReqInQueue;
    firstReqInQueue = getRequest();
    if (firstReqInQueue != NULL) {
        findBestElev(firstReqInQueue);
    }

    // Check if the connection with the best elevator is available
    // if so send message to the best elevator
    // else remove it from the available elevators list
}

//int floor;
//Button button; 

void findBestElev(Request request){
    int floorOfReq = request.floor;
    Button pressedButton = request.button;

    Elevator elev_states[MAX_ELEVATORS];

    char bestElevatorIP[32] = 0;

    for (int i = 0; i < MAX_ELEVATORS; i++) {
        Elevator currentElevator = readElevator(available_elevators[i]);
        if (currentElevator.floor)
    }
    
    // If one elevator is currently idle in the requested floor, select that one
    // else if the pressed button is B_HallDown check if an elevator above is going down, consider that one (take the floor)
	// if there's is an idle elevator closer, select that one
	// else, select the one that was checked in first place
    // else if the pressed button is B_HallUp check if an elevator below is going up, consider that one (take the floor)
	// if there's is an idle elevator closer, select that one
	// else, select the one that was checked in first place
}


