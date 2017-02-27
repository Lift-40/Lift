#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "fsm.h"
#include "con_load.h"
#include "elevator.h"
#include "elevator_io_device.h"
#include "requests.h"
#include "timer.h"
#include "../network_driver/network_io.h"
#include "../configuration.h"

static Message msg;
static Elevator             elevator;
static ElevOutputDevice     outputDevice;
extern char serverIP[32];

static void __attribute__((constructor)) fsm_init(){
    elevator = elevator_uninitialized();
    
    con_load("elevator.con",
        con_val("doorOpenDuration_s", &elevator.config.doorOpenDuration_s, "%lf")
        con_enum("clearRequestVariant", &elevator.config.clearRequestVariant,
            con_match(CV_All)
            con_match(CV_InDirn)
        )
    )
	
	strcpy(msg.senderIP, getMyIP());
	strcpy(elevator.ip, getMyIP());
	// msg.senderIP = getMyIP();
	strcpy(msg.destinationIP, serverIP);
	// msg.destinationIP = serverIP;
	msg.role = elev;
	msg.isEmpty = false;
    
    outputDevice = elevio_getOutputDevice();
}

static void setAllLights(Elevator es){
    for(int floor = 0; floor < N_FLOORS; floor++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            outputDevice.requestButtonLight(floor, btn, es.requests[floor][btn]);
        }
    }
}

void fsm_onInitBetweenFloors(void){
	outputDevice.motorDirection(D_Down);
	elevator.dirn = D_Down;
	elevator.behaviour = EB_Moving;
	strcpy(msg.destinationIP, serverIP);
	//msg.destinationIP = serverIP;
	msg.type = elev_state;
	Request emptyRequest;
	emptyRequest.floor = NUM_FLOORS + 1;
	emptyRequest.isEmpty = true;
	msg.request = emptyRequest;
	msg.elev_struct = elevator;
	if (serverIP[0] != 0) {
		printf("Sending elevetor structure to the server, onInitBetweenFloors\n");
		sendMessage(msg);
		printf("Done\n");
	}
}

void fsm_onRequestButtonPress(int btn_floor, Button btn_type, bool reqIsFromServer){
    printf("\n\n%s(%d, %s)\n", __FUNCTION__, btn_floor, elevio_button_toString(btn_type));
    elevator_print(elevator);
	
	printf("(fsm.c)connectionAvailable(serverIP): %i\n", connectionAvailable(serverIP));
    
	if (reqIsFromServer || !connectionAvailable(serverIP)) {
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
    
    setAllLights(elevator);
    //strcpy(msg.destinationIP, serverIP);
    printf("\nNew state:\n");
    elevator_print(elevator);
	strcpy(msg.destinationIP, serverIP);
	msg.type = elev_state;
	Request emptyRequest;
	emptyRequest.floor = NUM_FLOORS + 1;
	emptyRequest.isEmpty = true;
	msg.request = emptyRequest;
	msg.elev_struct = elevator;
	if (serverIP[0] != 0) {
		printf("(fsm.c)Sending elevator structure to the server, OnReqButtonPress\n");
		sendMessage(msg);
		//printf("Done\n");
	}	
	if (!reqIsFromServer) {
		printf("(fsm.c)Got an internal request, attempting to send order ( %d, %02x ) to the server\n", btn_floor, btn_type);
		// order is from one of the elevator buttons, send it to the server
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
			//printf("Done\n");
		}
	}
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
            timer_start(elevator.config.doorOpenDuration_s);
            setAllLights(elevator);
            elevator.behaviour = EB_DoorOpen;
        }
        break;
    default:
        break;
    }
    
    printf("\nNew state:\n");
    elevator_print(elevator);
	strcpy(msg.destinationIP, serverIP);
	printf("Server IP: %s\n", serverIP);
	//msg.destinationIP = serverIP;
	msg.type = elev_state;
	Request emptyRequest;
	emptyRequest.floor = NUM_FLOORS + 1;
	emptyRequest.isEmpty = true;
	msg.request = emptyRequest;
	msg.elev_struct = elevator;
	if (serverIP[0] != 0) {
		printf("Sending elevetor structure to the server, onFloorArrival\n");
		sendMessage(msg);
		//printf("Done\n");
	}
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
	// msg.destinationIP = serverIP;
	msg.type = elev_state;
	Request emptyRequest;
	emptyRequest.floor = NUM_FLOORS + 1;
	emptyRequest.isEmpty = true;
	msg.request = emptyRequest;
	msg.elev_struct = elevator;
	if (serverIP[0] != 0) {
		printf("Sending elevetor structure to the server, onDoorTimeout\n");
		sendMessage(msg);
		//printf("Done\n");
	}
}













