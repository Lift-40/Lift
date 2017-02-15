
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "con_load.h"
#include "elevator_io_device.h"
#include "../network_driver/network_io.h"
#include "fsm.h"
#include "timer.h"

int main(void){
    printf("Started!\n");
    
    int inputPollRate_ms = 25;
    con_load("elevator.con",
        con_val("inputPollRate_ms", &inputPollRate_ms, "%d")
    )
	
	networkInit();
    
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
			if (msg != NULL) {
				// if it's a request type then add it to the queue         
				if (msg.type == req) {
					fsm_onRequestButtonPress(msg.request.floor, msg.request.button);
				}
				// The light message type needs to be handled here
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









