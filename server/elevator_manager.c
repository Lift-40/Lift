#include "elevator_manager.h"
#include "../elev_algo/elevator_io_types.h"
#include "../network_driver/network_io.h"
#include "elevator_storage.h"
#include "queue.h"
#include "../configuration.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char available_elevators[MAX_ELEVATORS][32];

/*-----------------------------INTERNAL FUNCIONS---------------------------------*/

char * findBestElev(Request request){
    int floorOfReq = request.floor;
    Button pressedButton = request.button;
	
	printReq(request);

    int stop = 0;

    Elevator elev_states[MAX_ELEVATORS];

    char bestElevatorIP[32] = "";

    for (int i = 0; i < MAX_ELEVATORS; i++) {
		if (available_elevators[i][0] != 0) {
			elev_states[i] = readElevator(available_elevators[i]);
		} 
		// If one elevator is currently idle in the requested floor, select that one    	
		if (!elev_states[i].isEmpty && elev_states[i].behaviour == EB_Idle && elev_states[i].floor == floorOfReq){
			strcpy(bestElevatorIP, elev_states[i].ip);
			// bestElevatorIP = elev_states[i].ip;
			stop = 1;
		}
    }
	
	printf("Available elevator states stored");

    int minFloorDiffIdle = INF;
    int minFloorDiffMov = INF;
    int floorDiff = 0;

    int indexIdle = 0;
    int indexMov = 0;
	
    if (stop == 0 && (pressedButton == B_HallDown || (floorOfReq == 0 && pressedButton == B_HallUp))) {

		for (int i = 0; i < MAX_ELEVATORS; i++) {
			if (!elev_states[i].isEmpty && elev_states[i].behaviour == EB_Idle) {

				floorDiff = elev_states[i].floor - floorOfReq;
				if (floorDiff > 0 && floorDiff < minFloorDiffIdle) {

					minFloorDiffIdle = floorDiff;
					indexIdle = i;

				}
			 }
			 else if (!elev_states[i].isEmpty && elev_states[i].behaviour == EB_Moving) {

				floorDiff = elev_states[i].floor - floorOfReq;
				if (floorDiff > 0 && floorDiff < minFloorDiffMov) {

					minFloorDiffMov = floorDiff;
					indexMov = i;

				}
			 }
		}
		if (minFloorDiffMov <= minFloorDiffIdle) {
			strcpy(bestElevatorIP, elev_states[indexMov].ip);
			// bestElevatorIP = elev_states[indexMov].ip;
		} else {
			strcpy(bestElevatorIP, elev_states[indexIdle].ip);
			// bestElevatorIP = elev_states[indexIdle];
		}
    }
    else if (stop == 0 && (pressedButton == B_HallUp || (floorOfReq == NUM_FLOORS-1 && pressedButton == B_HallDown))) {

    	for (int i = 0; i < MAX_ELEVATORS; i++) {
			if (!elev_states[i].isEmpty && elev_states[i].behaviour == EB_Idle) {

				floorDiff = -(elev_states[i].floor - floorOfReq);
				if (floorDiff > 0 && floorDiff < minFloorDiffIdle) {

					minFloorDiffIdle = floorDiff;
					indexIdle = i;

				}
	    	}
            else if (!elev_states[i].isEmpty && elev_states[i].behaviour == EB_Moving) {
		    
				floorDiff = -(elev_states[i].floor - floorOfReq);
				if (floorDiff > 0 && floorDiff < minFloorDiffMov) {

					minFloorDiffMov = floorDiff;
					indexMov = i;

				}
			}
        }
		if (minFloorDiffMov <= minFloorDiffIdle) {
			strcpy(bestElevatorIP, elev_states[indexMov].ip);
            // bestElevatorIP = elev_states[indexMov];
        } else {
			strcpy(bestElevatorIP, elev_states[indexIdle].ip);
	    	// bestElevatorIP = elev_states[indexIdle];
        }	
    }
	return bestElevatorIP;
}

void initElevs(){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        available_elevators[i][0] = '\0';
    }
}

void addElev(const char * ip){
    int exists = 0;
	int added = 0;
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(strcmp( ip, available_elevators[i]) == 0){
	    printf("The ip %s already exists in available_elevators\n",ip);
	    exists = 1;
	}
    }
    if (exists == 0){
    	for(int i = 0; i < MAX_ELEVATORS; i++){
            if(available_elevators[i][0] == 0 && added == 0) {
                printf("Adding elevator %s to index %d\n",ip,i);
                strcpy( available_elevators[i], ip);
				added = 1;
            }
        }
    }
}

void removeElev(const char * ip){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(strcmp( ip, available_elevators[i] ) == 0){
            printf("Removing elevator %s from available_elevators %d\n", ip, i);
            available_elevators[i][0] = 0;
            return;
        }
    }
}

/*-----------------------------EXTERNAL FUNCIONS---------------------------------*/

int server_init() {
	initElevs();
	initStates();
    networkInit(4041, server);
	broadcastIP(server);
}

// TODO: Fill the available_elevators array with the ips of the working elevator

int server_routine() {	
    /* code goes here */
    Message msg;
    // Call receive message
    msg = receiveMessage();
    if (msg.isEmpty == false) {
		printf("Received new message\n");
		// if it's a request type then add it to the queue
		addElev(msg.senderIP);
		if (msg.type == req){
			printf("Message type is request, adding to queue\n");
	    	storeRequest(msg.request);
		}
		// else if it's a state type then add it to the storage module
		else if (msg.type == elev_state) {
			printf("Message type is elev_state, adding to storage\n");
			storeElevator(msg.elev_struct);
	    	// TODO:if state type, check if current first order of the queue is in the elevator's queue 
	    	// if it is, remove order from server queue
    	}
		// There's no need to handle the light type since those orders will only be sent to the elevators
	}
	
	Request firstReqInQueue;
    firstReqInQueue = getRequest();
    if (firstReqInQueue.isEmpty == false) {
		printf("Attempting to process request %d, %d\n", firstReqInQueue.floor, firstReqInQueue.button);
		char bestElevIP[32] = "";
        strcpy(bestElevIP, findBestElev(firstReqInQueue));
		printf("Best elevator found: %s\n", bestElevIP);
		Elevator emptyElevator;
		Message msg;
		strcpy(msg.senderIP, getMyIP());
		strcpy(msg.destinationIP, bestElevIP);
		msg.type = req;
		msg.request = firstReqInQueue;
		msg.elev_struct = emptyElevator;
		msg.role = server;
		msg.isEmpty = false;
		//Message msg = {getMyIP(), bestElevIP, req, firstReqInQueue, emptyElevator, server, false};
		// Check if the connection with the best elevator is available
		if (connectionAvailable(bestElevIP)){
			// if so send message to the best elevator
			printf("Request sent to best elevator with IP %s\n", bestElevIP);
			sendMessage(msg);
		} else {
			// else remove it from the available elevators list
			removeElev(bestElevIP);
		}
    }
}
