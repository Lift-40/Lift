
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "con_load.h"
#include "elevator_io_device.h"
#include "../network_driver/network_io.h"
#include "fsm.h"
#include "timer.h"

char serverIP[32] = "";

int main(int argc, char *argv[]){
    printf("Started!\n");
    
    int inputPollRate_ms = 25;
    con_load("elevator.con",
        con_val("inputPollRate_ms", &inputPollRate_ms, "%d")
    )
		
	if (argc != 2){
		printf("Port number not defined or invalid.");
		exit(1);
	}
	
	networkInit(atoi(argv[1]), elev);
	
    ElevInputDevice input = elevio_getInputDevice();
	    
    if(input.floorSensor() == -1){
        fsm_onInitBetweenFloors();
    }
        
    while(1){
        { // Request button
            static int prev[N_FLOORS][N_BUTTONS];
            for(int f = 0; f < N_FLOORS; f++){
                for(int b = 0; b < N_BUTTONS; b++){
                    int v = input.requestButton(f, b);
                    if(v  &&  v != prev[f][b]){
                        fsm_onRequestButtonPress(f, b);
                    }
                    prev[f][b] = v;
                }
            }
        }
		
		{ // Network
			Message msg;
			// Call receive message
			msg = receiveMessage();
			if (msg.isEmpty == false && msg.role == server) {
				// if it's a request type then add it to the queue
				printf("Message received\n");
				strcpy(serverIP, msg.senderIP);
				printf("Server IP updated to %s\n", msg.senderIP);
				if (msg.type == req) {
					fsm_onRequestButtonPress(msg.request.floor, msg.request.button);
				}
				// TODO: The light message type needs to be handled here
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
    }
}









