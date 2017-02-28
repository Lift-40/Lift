typedef struct {
	Elevator elev_states[MAX_ELEVATORS];
    Queued_Request queue[100000];
} Server;

int server_init();
int server_routine();
