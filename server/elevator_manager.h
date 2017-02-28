#include "../elevator/elevator.h"
#include "../configuration.h"
#include "queue.h"

typedef struct {
	Elevator elev_states[MAX_ELEVATORS];
    Queued_Req queue[100000];
} Server;

int server_init();
int server_routine();