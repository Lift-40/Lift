#include "elevator_manager.h"
#include "../configuration.h"
#include "../network_driver/network_io.h"
#include <time.h>

int main() {
    server_init();
	
	unsigned int prevTime = time(0);
		
	while(1){
		server_routine();
		if (time(0) > prevTime + 3){
			broadcastIP(server);
			prevTime = time(0);
		}
	}
}
