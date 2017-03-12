
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../configuration.h"

#include "con_load.h"
#include "elevator_io_device.h"
#include "../drivers/network_io.h"
#include "fsm.h"
#include "timer.h"
#include <time.h>


char serverIP[32] = "";

//int elevatorID = -1;

extern Elevator elevator;

int main(int argc, char *argv[]){
    printf("Started!\n");
    
    int inputPollRate_ms = 25;
    con_load("elevator.con",
        con_val("inputPollRate_ms", &inputPollRate_ms, "%d")
    )
		
	if (argc != 2){
		printf("Port number not defined or invalid.\n");
		exit(1);
	}
	
	int elevatorID = atoi(argv[1]);
	setFSM(elevatorID);
	printf("Attempting to initiate network\n");
	networkInit(elevatorID, elev);
	printf("Network started\n");
	
    ElevInputDevice input = elevio_getInputDevice();
	    
    if(input.floorSensor() == -1){
        fsm_onInitBetweenFloors();
    }
	
	unsigned int prevTime = time(0);
        
    while(1){
		
		if (time(0) > prevTime + 3){
			Message msg;
			strcpy(msg.destinationIP, serverIP);
			strcpy(msg.senderIP, getMyIP());
			msg.type = elev_state;
			msg.elevatorID = elevatorID;
			msg.role = elev;
			Request emptyRequest;
			emptyRequest.floor = NUM_FLOORS + 1;
			emptyRequest.isEmpty = true;
			msg.request = emptyRequest;
			msg.isEmpty = false;
			msg.elev_struct = elevator;
			if (serverIP[0] != 0) {
				printf("(main.c)Sending elevator structure to the server, timer\n");
				sendMessage(msg);
			}	
			prevTime = time(0);
		}
		
		printf("(fsm.c)connectionAvailable(serverIP): %i\n", connectionAvailable(serverIP));
		printf("ServerIP: %s\n", serverIP);
		if (!connectionAvailable(serverIP) && serverIP[0] != 0) {
			printf("Attempting to connect to server\n");
			Message msg1;
			msg1.type = broadcast;
			msg1.elevatorID = elevatorID;
			msg1.role = elev;
			msg1.isEmpty = false;
			strcpy(msg1.destinationIP, serverIP);
			strcpy(msg1.senderIP, getMyIP());
			sendMessage(msg1);
		}
		
        { // Request button
            static int prev[N_FLOORS][N_BUTTONS];
            for(int f = 0; f < N_FLOORS; f++){
                for(int b = 0; b < N_BUTTONS; b++){
                    int v = input.requestButton(f, b);
                    if(v  &&  v != prev[f][b]){
                        fsm_onRequestButtonPress(f, b, false);
                    }
                    prev[f][b] = v;
                }
            }
        }
		
		{ // Network
			Message msg2;
			// Call receive message
			msg2 = receiveMessage();
			if (msg2.isEmpty == false && msg2.role == server) {
				// if it's a request type then add it to the queue
				printf("Message received by the elevator\n");
				strcpy(serverIP, msg2.senderIP);
				printf("Server IP updated to %s\n", msg2.senderIP);
				if (msg2.type == req && msg2.request.floor < NUM_FLOORS+1 && msg2.request.button < 3) {
					fsm_onRequestButtonPress(msg2.request.floor, msg2.request.button, true);
				}
				// TODO: THE LIGHT MESSAGE TYPE NEEDS TO BE HANDLE HERE
				if (msg2.type == light_update){
					fsm_lightUpdating(msg2.request.floor, msg2.request.button);
				}
				if (msg2.type == light_update_onFloorArrival){
					fsm_lightUpdating_onFloorArrival( msg2.elev_struct.floor );
				}
			}
		}
        
        { // Floor sensor
            static int prev;
            int f = input.floorSensor();
            if(f != -1  &&  f != prev){
                fsm_onFloorArrival(f);
            }
            prev = f;
        }
        
        
        { // Timer
            if(timer_timedOut()){
                fsm_onDoorTimeout();
                timer_stop();
            }
        }
        
        usleep(inputPollRate_ms*1000);
		printf("\n-------------------------------------\n");
    }
}









