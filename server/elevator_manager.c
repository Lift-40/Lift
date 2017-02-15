#include "elevator_manager.h"
#include "elevator_io_types.h"
#include "../network_driver/network_io.h"
#include "queue.h"
#include "../configuration.h"

#include <stdio.h>
#include <stdlib.h>

char available_elevators[MAX_ELEVATORS][32];

/*-----------------------------INTERNAL FUNCIONS---------------------------------*/

char findBestElev(Request request){
    int floorOfReq = request.floor;
    Button pressedButton = request.button;

    int stop = 0;

    Elevator elev_states[MAX_ELEVATORS];

    char bestElevatorIP[32] = 0;

    for (int i = 0; i < MAX_ELEVATORS; i++) {
        Elevator elev_states[i] = readElevator(available_elevators[i]);
		// If one elevator is currently idle in the requested floor, select that one    	
		if (elev_states[i].behaviour == EB_Idle && elev_states[i].floor == floorOfReq){
			bestElevatorIP = elev_states[i].ip;
			stop = 1;
		}
    }

    int minFloorDiffIdle = INF;
    int minFloorDiffMov = INF;
    int floorDiff = 0;

    int indexIdle = 0;
    int indexMov = 0;
	
    if (stop == 0 && (pressedButton == B_HallDown || (floorOfReq == 0 && pressedButton == B_HallUp))) {

		for (int i = 0; i < MAX_ELEVATORS; i++) {
			if (elev_states[i].behaviour == EB_Idle) {

				floorDiff = elev_states[i].floor - floorOfReq;
				if (floorDiff > 0 && floorDiff < minFloorDiffIdle) {

					minFloorDiffIdle = floorDiff;
					indexIdle = i;

				}
			 }
			 else if (elev_states[i].behaviour == EB_Moving) {

				floorDiff = elev_states[i].floor - floorOfReq;
				if (floorDiff > 0 && floorDiff < minFloorDiffMov) {

					minFloorDiffMov = floorDiff;
					indexMov = i;

				}
			 }
		}
		if (minFloorDiffMov <= minFloorDiffIdle) {
			bestElevatorIP = elev_states[indexMov];
		} else {
			bestElevatorIP = elev_states[indexIdle];
		}
    }
    else if (stop == 0 && (pressedButton == B_HallUp || (floorOfReq == NUM_FLOORS-1 && pressedButton == B_HallDown))) {

    	for (int i = 0; i < MAX_ELEVATORS; i++) {
			if (elev_states[i].behaviour == EB_Idle) {

				floorDiff = -(elev_states[i].floor - floorOfReq);
				if (floorDiff > 0 && floorDiff < minFloorDiffIdle) {

					minFloorDiffIdle = floorDiff;
					indexIdle = i;

				}
	    	}
            else if (elev_states[i].behaviour == EB_Moving) {
		    
				floorDiff = -(elev_states[i].floor - floorOfReq);
				if (floorDiff > 0 && floorDiff < minFloorDiffMov) {

					minFloorDiffMov = floorDiff;
					indexMov = i;

				}
			}
        }
		if (minFloorDiffMov <= minFloorDiffIdle) {
            bestElevatorIP = elev_states[indexMov];
        } else {
	    	bestElevatorIP = elev_states[indexIdle];
        }	
    }
	return bestElevatorIP;
}

void initElevs(){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        available_elevators[i][0] = 0;
    }
}

void removeElev(const char * ip){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(strncmp( ip, available_elevators[i], strlen(available_elevators[i]) ) == 0 && available_elevators[i][0] != 0){
            printf("Removing elevator %s from available_elevators %d\n", ip, i);
            available_elevators[i][0] = 0;
            return;
        }
    }
}

/*-----------------------------EXTERNAL FUNCIONS---------------------------------*/

int server_init() {
	initElevs();
    networkInit();
}

// TODO: Fill the available_elevators array with the ips of the working elevator

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
		char bestElevIP[32] = 0;
        bestElevIP = findBestElev(firstReqInQueue);
		Message msg = {getMyIP(), bestElevIP, req, firstReqInQueue, NULL};
		// Check if the connection with the best elevator is available
		if (connectionAvailable(bestElevIP)){
			// if so send message to the best elevator
			sendMessage(msg);
		} else {
			// else remove it from the available elevators list
			removeElev(bestElevIP);
		}
    }
}
