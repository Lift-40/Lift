typedef struct {
	Elevator elev_states[MAX_ELEVATORS]
    Queued_Request queue[100000];
    
	bool isEmpty;
} Server;

int server_init();
int server_routine();
