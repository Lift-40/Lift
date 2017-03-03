#include "elevator_manager.h"
#include "../elevator/elevator_io_types.h"
#include "../drivers/network_io.h"
#include "elevator_storage.h"
#include "queue.h"
#include "../configuration.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int available_elevators[MAX_ELEVATORS];
Server serverState;

/*-----------------------------INTERNAL FUNCIONS---------------------------------*/

int findBestElev(Request request){
    int floorOfReq = request.floor;
    Button pressedButton = request.button;
	
	printf("\nFind best elevator for request:");
	printReq(request);

    int stop = 0;

    Elevator elev_States_Server[MAX_ELEVATORS];
	for(int i = 0; i < MAX_ELEVATORS; i++){
		Elevator emptyElevator;
		emptyElevator.isEmpty = true;
		emptyElevator.ip[0] = 0;
		emptyElevator.elevatorID = 0;
		memcpy(&elev_States_Server[i], &emptyElevator, sizeof(Elevator));
    }

    int bestElevatorID = 0;

    for (int i = 0; i < MAX_ELEVATORS; i++) {
		if (available_elevators[i] != 0) {
			Elevator data;
			data = readElevator(available_elevators[i]);
			//printf("Read elevator\n");
			memcpy( &elev_States_Server[i], &data, sizeof(Elevator) );
			printf("(elevator_manager.c)Getting elevator struct from storage: %i\n",i);
			printf("(elevator_manager.c)elev_struct.isEmpty: %i\n",elev_States_Server[i].isEmpty);
		}
		// If one elevator is currently idle in the requested floor, select that one    	
		if (!elev_States_Server[i].isEmpty && (elev_States_Server[i].behaviour == EB_Idle || elev_States_Server[i].behaviour == EB_DoorOpen) && elev_States_Server[i].floor == floorOfReq){
			printf("(elevator_manager.c)Best elevator is idle on floor: %i\n",i);
			bestElevatorID = elev_States_Server[i].elevatorID;
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
			if (!elev_States_Server[i].isEmpty && (elev_States_Server[i].behaviour == EB_Idle || elev_States_Server[i].behaviour == EB_DoorOpen)) {
				
				floorDiff = abs(elev_States_Server[i].floor - floorOfReq);
				if (floorDiff < minFloorDiffIdle) {

					minFloorDiffIdle = floorDiff;
					indexIdle = i;

				}
			 }
			 else if (!elev_States_Server[i].isEmpty && elev_States_Server[i].behaviour == EB_Moving) {

				floorDiff = elev_States_Server[i].floor - floorOfReq;
				if (floorDiff > 0 && floorDiff < minFloorDiffMov) {

					minFloorDiffMov = floorDiff;
					indexMov = i;

				}
			 }
		}
		if (minFloorDiffMov <= minFloorDiffIdle && indexMov != -1) {
			bestElevatorID = elev_States_Server[indexMov].elevatorID;
		} else if (indexIdle != -1) {
			bestElevatorID = elev_States_Server[indexIdle].elevatorID;
		}
    }
    else if (stop == 0 && (pressedButton == B_HallUp || (floorOfReq == NUM_FLOORS-1 && pressedButton == B_HallDown))) {
		//printf("Didnt stop, pressedButton: Up or floor = 3 && pressedButton: Down\n");
    	for (int i = 0; i < MAX_ELEVATORS; i++) {
			if (!elev_States_Server[i].isEmpty && (elev_States_Server[i].behaviour == EB_Idle || elev_States_Server[i].behaviour == EB_DoorOpen)) {

				floorDiff = abs(elev_States_Server[i].floor - floorOfReq);
				if (floorDiff < minFloorDiffIdle) {

					minFloorDiffIdle = floorDiff;
					indexIdle = i;

				}
	    	}
            else if (!elev_States_Server[i].isEmpty && elev_States_Server[i].behaviour == EB_Moving) {
		    
				floorDiff = -(elev_States_Server[i].floor - floorOfReq);
				if (floorDiff > 0 && floorDiff < minFloorDiffMov) {

					minFloorDiffMov = floorDiff;
					indexMov = i;

				}
			}
        }
		if (minFloorDiffMov <= minFloorDiffIdle && indexMov != -1) {
			bestElevatorID = elev_States_Server[indexMov].elevatorID;
        } else if (indexIdle != -1) {
			bestElevatorID = elev_States_Server[indexIdle].elevatorID;
        }	
    }
	if(((indexMov == -1) && (indexIdle == -1) ) && stop == 0) {
		bestElevatorID = 0;
	}
	
	return bestElevatorID;
}

void initElevs(){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        available_elevators[i] = 0;
    }
}

void addElev(int elevatorID){
    int exists = 0;
	int added = 0;
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(available_elevators[i] == elevatorID){
			printf("(elevator_manager.c)The ID of the elevator %d already exists in server\n",elevatorID);
			exists = 1;
		}
    }
    if (exists == 0){
    	for(int i = 0; i < MAX_ELEVATORS; i++){
            if(available_elevators[i] == 0 && added == 0) {
                printf("(elevator_manager.c)Adding elevator ID %d to index %d in server\n",elevatorID,i);
                available_elevators[i] = elevatorID;
				added = 1;
            }
        }
    }
}

void removeElev(int elevatorID){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(available_elevators[i] == elevatorID) {
            printf("(elevator_manager.c)Removing elevator %d from available_elevators in server%d\n", elevatorID, i);
            available_elevators[i] = 0;
            return;
        }
    }
}

void updateServerStruct() {	
	serverState.isValid = false;
	
	QueueArray tmp;
	
	tmp = get_Requests_Array();
	
	serverState.queueLength = tmp.length;
	
	memcpy(serverState.queue, tmp.queue, tmp.length);
	
	// store elevator state backups
	for(int i = 0; i < MAX_ELEVATORS; i++) {
		serverState.elev_states[i] = readElevatorByIndex(i);
	}
	
	serverState.isValid = true;
}

/*-----------------------------EXTERNAL FUNCIONS---------------------------------*/

int server_init() {
	initElevs();
	initStates();
	updateServerStruct();
	Server *loadedBackup;
	if (( loadedBackup = (Server *)malloc( sizeof(Server) ) ) == NULL) {
		printf("(elevator_manager.c)Out of memory for server!\n");
        return 0;
	}
	
	loadedBackup = loadServerBackup();
	if (loadedBackup -> isValid) {
		// restore queue backup
		printf("(elevator_manager.c)Attempting to restore backup\n");
		for(int i = 0; i < loadedBackup -> queueLength; i++) {
			printf("(elevator_manager.c)Loaded order %d\n", i);
			storeRequest(loadedBackup -> queue[i]);
		}
		// restore elevator state backups
		for(int i = 0; i < MAX_ELEVATORS; i++) {
			printf("(elevator_manager.c)Loaded elev state %d\n", i);
			storeElevator(loadedBackup -> elev_states[i]);
		}
	}
    networkInit(0, server);
	broadcastIP(server, 0);
}

// TODO: Fill the available_elevators array with the ips of the working elevator

int server_routine() {	
	updateServerStruct();
    /* code goes here */
    Message msg;
    // Call receive message
    msg = receiveMessage();
    if (msg.isEmpty == false) {
		printf("(elevator_manager.c)Server receives new message\n");
		// if it's a request type then add it to the queue
		addElev(msg.elevatorID);
		if (msg.type == req){
			printf("(elevator_manager.c)Message type is request, adding to server queue\n");
	    	storeRequest(msg.request);
			writeServerBackup(&serverState);
		}
		// else if it's a state type then add it to the storage module
		else if (msg.type == elev_state) {
			printf("(elevator_manager.c)Message type is elev_state, adding to server storage\n");
			storeElevator(msg.elev_struct);
			writeServerBackup(&serverState);
	    				
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
    	} /*else if (msg.type == broadcast) {
			addElev(msg.elevatorID);
		}*/
		// There's no need to handle the light type since those orders will only be sent to the elevators
	}
	
	Request firstReqInQueue;
    firstReqInQueue = getRequest();
    if (firstReqInQueue.isEmpty == false) {
		printf("(elevator_manager.c)Attempting to process request %d, %d\n", firstReqInQueue.floor, firstReqInQueue.button);
		int bestElevID = findBestElev(firstReqInQueue);
		if (bestElevID == 0){
			printf("(elevator_manager.c)Could not fint best elevator\n");
		}
		else{
			printf("(elevator_manager.c)Best elevator found: %d\n", bestElevID);
			Elevator emptyElevator;
			Message msg;
			strcpy(msg.senderIP, getMyIP());
			strcpy(msg.destinationIP, readElevator(bestElevID).ip);
			msg.type = req;
			msg.request = firstReqInQueue;
			msg.elevatorID = bestElevID;
			msg.elev_struct = emptyElevator;
			msg.role = server;
			msg.isEmpty = false;
			//Message msg = {getMyIP(), bestElevIP, req, firstReqInQueue, emptyElevator, server, false};
			// Check if the connection with the best elevator is available
			if (connectionAvailable(readElevator(bestElevID).ip) && bestElevID != 0){
				// if so send message to the best elevator
				printf("(elevator_manager.c)After finding best elevator with ID %d, send request\n", bestElevID);
				sendMessage(msg);
			} else {
				// else remove it from the available elevators list
				removeElev(bestElevID);
			}
		}
    }
	printf("\n------------------------------------------------\n");
	sleep(1);
}
