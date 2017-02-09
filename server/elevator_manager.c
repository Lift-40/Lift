#include "elevator_manager.h"
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
    // TODO:
}


