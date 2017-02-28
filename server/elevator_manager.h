#include "../elevator/elevator.h"
#include "../configuration.h"
#include "queue.h"
#include <stdbool.h>

typedef struct {
	Elevator elev_states[MAX_ELEVATORS];
	int queueLength;
    Request queue[MAX_QUEUE_BACKUPS];
	bool isValid;
} Server;

int server_init();
int server_routine();