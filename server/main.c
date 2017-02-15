#include "elevator_manager.h"
#include "../configuration.h"

int main() {
    server_init();
	while(1){
		server_routine();
	}
}
