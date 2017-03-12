#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "fsm.h"
#include "con_load.h"
#include "elevator.h"
#include "elevator_io_device.h"
#include "requests.h"
#include "timer.h"
#include "../drivers/network_io.h"
#include "../drivers/backup.h"
#include "../configuration.h"

static Message msg;
Elevator             elevator;
static ElevOutputDevice     outputDevice;
extern char serverIP[32];
int elevatorID;
int requestsMade[N_FLOORS][N_BUTTONS];

static void __attribute__((constructor)) fsm_init(){
    elevator = elevator_uninitialized();
    
    con_load("elevator.con",
        con_val("doorOpenDuration_s", &elevator.config.doorOpenDuration_s, "%lf")
        con_enum("clearRequestVariant", &elevator.config.clearRequestVariant,
            con_match(CV_All)
            con_match(CV_InDirn)
        )
    )
	
    outputDevice = elevio_getOutputDevice();
	
	for(int floor = 0; floor < N_FLOORS; floor++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            requestsMade[floor][btn] = 0;
        }
    }
}

void setFSM(int elevID){
	Elevator *loadedBackup;
	if (( loadedBackup = (Elevator *)malloc( sizeof(Elevator) ) ) == NULL) {
		printf("(elevator_manager.c)Out of memory for server!\n");
        return 0;
	}
	loadedBackup = loadElevatorBackup(elevID);
		
	elevator.elevatorID = loadedBackup -> elevatorID;
	strcpy(elevator.ip, loadedBackup -> ip);
	elevator.floor = loadedBackup -> floor;
	elevator.dirn = loadedBackup -> dirn;
	
	memcpy(elevator.requests, loadedBackup -> requests, 10 * sizeof(int));
	
	elevator.behaviour = loadedBackup -> behaviour;
	elevator.config = loadedBackup -> config;
	elevator.isEmpty = loadedBackup -> isEmpty;	
	
	strcpy(msg.senderIP, getMyIP());
	strcpy(elevator.ip, getMyIP());
	strcpy(msg.destinationIP, serverIP);
	elevatorID = elevID;
	msg.role = elev;
	msg.isEmpty = false;	
	elevator.elevatorID = elevID;
	msg.elevatorID = elevID;
	writeElevatorBackup(&elevator, elevatorID);
}

static void setAllLights(Elevator es){
    for(int floor = 0; floor < N_FLOORS; floor++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            outputDevice.requestButtonLight(floor, btn, es.requests[floor][btn]);
        }
    }
}

static void setAllLightsForRequestsMade(){
    for(int floor = 0; floor < N_FLOORS; floor++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            outputDevice.requestButtonLight(floor, btn, requestsMade[floor][btn]);
        }
    }
}

void fsm_onInitBetweenFloors(void){
	outputDevice.motorDirection(D_Down);
	elevator.dirn = D_Down;
	elevator.behaviour = EB_Moving;
	strcpy(msg.destinationIP, serverIP);
	msg.type = elev_state;
	Request emptyRequest;
	emptyRequest.floor = NUM_FLOORS + 1;
	emptyRequest.isEmpty = true;
	elevator.elevatorID = elevatorID;
	msg.elevatorID = elevatorID;
	msg.request = emptyRequest;
	msg.elev_struct = elevator;
	if (serverIP[0] != 0) {
		printf("Sending elevetor structure to the server, onInitBetweenFloors\n");
		sendMessage(msg);
		printf("Done\n");
	}
	writeElevatorBackup(&elevator, elevatorID);
}

void fsm_lightUpdating(int btn_floor, Button btn_type){
	
	requestsMade[btn_floor][btn_type] = 1;

	setAllLightsForRequestsMade();
}

void fsm_lightUpdating_onFloorArrival( int floor ){
	
	for(int btn = 0; btn < N_BUTTONS; btn++){
		requestsMade[floor][btn] = 0;
	}
	setAllLightsForRequestsMade();
}

void fsm_onRequestButtonPress(int btn_floor, Button btn_type, bool reqIsFromServer){
    printf("\n\n%s(%d, %s)\n", __FUNCTION__, btn_floor, elevio_button_toString(btn_type));
    elevator_print(elevator);
	
	printf("(fsm.c)connectionAvailable(serverIP): %i\n", connectionAvailable(serverIP));
	
	// In order to save all the requests in backup, otherwise we would be only saving the last request.
	Elevator *loadedBackup;
	if (( loadedBackup = (Elevator *)malloc( sizeof(Elevator) ) ) == NULL) {
		printf("(elevator_manager.c)Out of memory for server!\n");
        return 0;
	}
	loadedBackup = loadElevatorBackup(elevatorID);
	memcpy(elevator.requests, loadedBackup -> requests, 10 * sizeof(int));
    
	if (reqIsFromServer || !connectionAvailable(serverIP) || btn_type == B_Cab) {
		if(elevator.floor == btn_floor && (elevator.behaviour == EB_DoorOpen || elevator.behaviour == EB_Idle)) {
			elevator.requests[btn_floor][btn_type] = 1;
			strcpy(msg.destinationIP, serverIP);
			msg.type = elev_state;
			Request emptyRequest;
			emptyRequest.floor = NUM_FLOORS + 1;
			emptyRequest.isEmpty = true;
			msg.request = emptyRequest;
			msg.elev_struct = elevator;
			if (serverIP[0] != 0) {
				printf("(fsm.c)Sending elevator structure to the server, ElevatorOnFloor\n");
				sendMessage(msg);
			}	
			elevator.requests[btn_floor][btn_type] = 0;
			
			for(int btn = 0; btn < N_BUTTONS; btn++){
				requestsMade[elevator.floor][btn] = 0;
			}
			setAllLightsForRequestsMade();
			
			msg.type = light_update_onFloorArrival;
			msg.elevatorID = elevatorID;
			if (serverIP[0] != 0) {
				printf("Sending light updating message from elevator %d to the server, elevatorOnFloor\n", elevatorID);
				sendMessage(msg);
			}
			
		}
	
		switch(elevator.behaviour){

		case EB_DoorOpen:
			if(elevator.floor == btn_floor){
				timer_start(elevator.config.doorOpenDuration_s);
			} else {
				elevator.requests[btn_floor][btn_type] = 1;
			}
			break;

		case EB_Moving:
			elevator.requests[btn_floor][btn_type] = 1;
			break;

		case EB_Idle:
			if(elevator.floor == btn_floor){
				outputDevice.doorLight(1);
				timer_start(elevator.config.doorOpenDuration_s);
				elevator.behaviour = EB_DoorOpen;
			} else {
				elevator.requests[btn_floor][btn_type] = 1;
				elevator.dirn = requests_chooseDirection(elevator);
				outputDevice.motorDirection(elevator.dirn);
				elevator.behaviour = EB_Moving;
			}
			break;
		}
	}
    
    if (!connectionAvailable(serverIP) || btn_type == B_Cab) {
		setAllLights(elevator);
	}
	
    printf("\nNew state:\n");
    elevator_print(elevator);
	strcpy(msg.destinationIP, serverIP);
	msg.type = elev_state;
	Request emptyRequest;
	elevator.elevatorID = elevatorID;
	msg.elevatorID = elevatorID;
	emptyRequest.floor = NUM_FLOORS + 1;
	emptyRequest.isEmpty = true;
	msg.request = emptyRequest;
	msg.elev_struct = elevator;
	if (serverIP[0] != 0) {
		printf("(fsm.c)Sending elevator structure to the server, OnReqButtonPress\n");
		sendMessage(msg);
	}	
	if (!reqIsFromServer && btn_type != B_Cab) {
		printf("(fsm.c)Got an internal request, attempting to send order ( %d, %02x ) to the server\n", btn_floor, btn_type);
		// Order is from one of the elevator buttons, send it to the server
		msg.type = req;
		Request request;
		request.floor = btn_floor;
		request.button = btn_type;
		request.isEmpty = false;
		msg.request = request;
		//msg.elev_struct = elevator;
		printf("(fsm.c)connectionAvailable(serverIP): %i\n", connectionAvailable(serverIP));
		if (serverIP[0] != 0 && connectionAvailable(serverIP)) {
			printf("(fsm.c)Sending elevator request to the server, OnReqButtonPress\n");
			sendMessage(msg);
		}
	}
	writeElevatorBackup(&elevator, elevatorID);
}

void fsm_onFloorArrival(int newFloor){
    printf("\n\n%s(%d)\n", __FUNCTION__, newFloor);
    elevator_print(elevator);
    
    elevator.floor = newFloor;
    
    outputDevice.floorIndicator(elevator.floor);
    
    switch(elevator.behaviour){
    case EB_Moving:
        if(requests_shouldStop(elevator)){
            outputDevice.motorDirection(D_Stop);
            outputDevice.doorLight(1);
            elevator = requests_clearAtCurrentFloor(elevator);
			//NEW
			printf("Before arriving\n");
			for(int floor = 0; floor < N_FLOORS; floor++){
				printf(" %d | %d | %d \n", requestsMade[floor][0], requestsMade[floor][1], requestsMade[floor][2]);
			}
			
			for(int btn = 0; btn < N_BUTTONS; btn++){
				requestsMade[elevator.floor][btn] = 0;
			}
			
			printf("After arriving\n");
			for(int floor = 0; floor < N_FLOORS; floor++){
				printf(" %d | %d | %d \n", requestsMade[floor][0], requestsMade[floor][1], requestsMade[floor][2]);
			}
			setAllLightsForRequestsMade();
			
            timer_start(elevator.config.doorOpenDuration_s);
			if (serverIP[0] == 0 || !connectionAvailable(serverIP)) {
				setAllLights(elevator);
			}
            
            elevator.behaviour = EB_DoorOpen;
        }
        break;
    default:
        break;
    }
    
    printf("\nNew state:\n");
    elevator_print(elevator);
	
	strcpy(msg.destinationIP, serverIP);
	msg.type = light_update_onFloorArrival;
	msg.elevatorID = elevatorID;
	Request emptyRequest;
	emptyRequest.floor = NUM_FLOORS + 1;
	emptyRequest.isEmpty = true;
	msg.request = emptyRequest;
	elevator.elevatorID = elevatorID;
	msg.elev_struct = elevator;
	if (serverIP[0] != 0) {
		printf("Sending light updating message from elevator %d to the server, onFloorArrival\n", elevatorID);
		sendMessage(msg);
	}	
	
	
	printf("Server IP: %s\n", serverIP);
	msg.type = elev_state;
	if (serverIP[0] != 0) {
		printf("Sending elevetor structure to the server, onFloorArrival\n");
		sendMessage(msg);
	}
	writeElevatorBackup(&elevator, elevatorID);
}

void fsm_onDoorTimeout(void){
    printf("\n\n%s()\n", __FUNCTION__);
    elevator_print(elevator);
    
    switch(elevator.behaviour){
    case EB_DoorOpen:
        elevator.dirn = requests_chooseDirection(elevator);
        
        outputDevice.doorLight(0);
        outputDevice.motorDirection(elevator.dirn);
        
        if(elevator.dirn == D_Stop){
            elevator.behaviour = EB_Idle;
        } else {
            elevator.behaviour = EB_Moving;
        }
        
        break;
    default:
        break;
    }
    
    printf("\nNew state:\n");
    elevator_print(elevator);
	strcpy(msg.destinationIP, serverIP);
	msg.type = elev_state;
	Request emptyRequest;
	emptyRequest.floor = NUM_FLOORS + 1;
	emptyRequest.isEmpty = true;
	msg.request = emptyRequest;
	elevator.elevatorID = elevatorID;
	msg.elevatorID = elevatorID;
	msg.elev_struct = elevator;
	if (serverIP[0] != 0) {
		printf("Sending elevetor structure to the server, onDoorTimeout\n");
		sendMessage(msg);
	}
	writeElevatorBackup(&elevator, elevatorID);
}













