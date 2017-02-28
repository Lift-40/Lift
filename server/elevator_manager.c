#include "elevator_manager.h"
#include "../elev_algo/elevator_io_types.h"
#include "../drivers/network_io.h"
#include "elevator_storage.h"
#include "queue.h"
#include "../configuration.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char available_elevators[MAX_ELEVATORS][32];
char lastBestElevatorIP[32];

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
			Elevator data;
			data = readElevator(available_elevators[i]);
			//printf("Read elevator\n");
			memcpy( &elev_states[i], &data, sizeof(Elevator) );
			printf("(elevator_manager.c)Getting elevator struct from storage: %i\n",i);
			printf("(elevator_manager.c)elev_struct.isEmpty: %i\n",elev_states[i].isEmpty);
		} 
		// If one elevator is currently idle in the requested floor, select that one    	
		if (!elev_states[i].isEmpty && elev_states[i].behaviour == EB_Idle && elev_states[i].floor == floorOfReq){
			printf("(elevator_manager.c)Best elevator is idle on floor: %i\n",i);
			strcpy(bestElevatorIP, elev_states[i].ip);
			stop = 1;
		}
    }
	
	//printf("Available elevator states stored\n");

    int minFloorDiffIdle = INF;
    int minFloorDiffMov = INF;
    int floorDiff = 0;

    int indexIdle = -1;
    int indexMov = -1;
	
    if (stop == 0 && (pressedButton == B_HallDown || (floorOfReq == 0 && pressedButton == B_HallUp))) {
		//printf("Didnt stop, pressedButton: Down or floor = 0 && pressedButton: Up\n");

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
		if (minFloorDiffMov <= minFloorDiffIdle && indexMov != -1) {
			strcpy(bestElevatorIP, elev_states[indexMov].ip);
			// bestElevatorIP = elev_states[indexMov].ip;
		} else if (indexIdle != -1) {
			strcpy(bestElevatorIP, elev_states[indexIdle].ip);
			// bestElevatorIP = elev_states[indexIdle];
		}
    }
    else if (stop == 0 && (pressedButton == B_HallUp || (floorOfReq == NUM_FLOORS-1 && pressedButton == B_HallDown))) {
		//printf("Didnt stop, pressedButton: Up or floor = 3 && pressedButton: Down\n");
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
		if (minFloorDiffMov <= minFloorDiffIdle && indexMov != -1) {
			strcpy(bestElevatorIP, elev_states[indexMov].ip);
            // bestElevatorIP = elev_states[indexMov];
        } else if (indexIdle != -1) {
			strcpy(bestElevatorIP, elev_states[indexIdle].ip);
	    	// bestElevatorIP = elev_states[indexIdle];
        }	
    }
	if(((indexMov == -1) && (indexIdle == -1) ) && stop != 1) {
		bestElevatorIP[0] = 0;
	} else if (indexIdle != -1 && stop == 0) {
		strcpy(bestElevatorIP, elev_states[indexIdle].ip);
	} else if (indexMov != -1 && stop  == 0) {
		strcpy(bestElevatorIP, elev_states[indexMov].ip);
	}
	char * buf = malloc(sizeof(char)*32);
	sprintf(buf, "%s", bestElevatorIP);
	return buf;
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
			printf("(elevator_manager.c)The IP of the elevator %s already exists in server\n",ip);
			exists = 1;
		}
    }
    if (exists == 0){
    	for(int i = 0; i < MAX_ELEVATORS; i++){
            if(available_elevators[i][0] == 0 && added == 0) {
                printf("(elevator_manager.c)Adding elevator IP %s to index %d in server\n",ip,i);
                strcpy( available_elevators[i], ip);
				added = 1;
            }
        }
    }
}

void removeElev(const char * ip){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(strcmp( ip, available_elevators[i] ) == 0){
            printf("(elevator_manager.c)Removing elevator %s from available_elevators in server%d\n", ip, i);
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
		printf("(elevator_manager.c)Server receives new message\n");
		// if it's a request type then add it to the queue
		addElev(msg.senderIP);
		if (msg.type == req){
			printf("(elevator_manager.c)Message type is request, adding to server queue\n");
	    	storeRequest(msg.request);
		}
		// else if it's a state type then add it to the storage module
		else if (msg.type == elev_state) {
			printf("(elevator_manager.c)Message type is elev_state, adding to server storage\n");
			storeElevator(msg.elev_struct);
	    				
			// Get first request in server queue
			printf("(elevator_manager.c)Getting first request from server queue\n");
			Request firstReqInQueue;
			firstReqInQueue = getRequest();
			
			int floor = firstReqInQueue.floor;
			int button = firstReqInQueue.button;
			
			// Find that order in the elevator's queue and remove it in the server queue
			if(msg.elev_struct.requests[floor][button] == 1){
				printf("(elevator_manager.c)Best elevator recevied order, removing first request from server queue\n");
				removeRequest();
			}

    	}
		// There's no need to handle the light type since those orders will only be sent to the elevators
	}
	
	Request firstReqInQueue;
    firstReqInQueue = getRequest();
    if (firstReqInQueue.isEmpty == false) {
		printf("(elevator_manager.c)Attempting to process request %d, %d\n", firstReqInQueue.floor, firstReqInQueue.button);
		char bestElevIP[32] = "";
        strcpy(bestElevIP, findBestElev(firstReqInQueue));
		if (bestElevIP[0] == 0){
			printf("(elevator_manager.c)Could not fint best elevator\n");
		}
		else{
			printf("(elevator_manager.c)Best elevator found: %s\n", bestElevIP);
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
			if (connectionAvailable(bestElevIP) && bestElevIP[0] != 0){
				// if so send message to the best elevator
				printf("(elevator_manager.c)After finding best elevator with IP %s, send request\n", bestElevIP);
				strcpy(lastBestElevatorIP, bestElevIP);
				sendMessage(msg);
			} else {
				// else remove it from the available elevators list
				removeElev(bestElevIP);
			}
		}
    }
	sleep(2);
}
