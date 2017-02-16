#include <stdbool.h>
#include "../elev_algo/elevator_io_types.h"
#include "../elev_algo/elevator.h"

typedef struct {
    int floor;
    Button button;
	bool isEmpty;
} Request;

typedef enum { 
    server,
	elevator
} senderRole;

typedef enum { 
    req,
    elev_state,
    light_update
} msgType;

typedef struct {
    char * senderIP;
	char * destinationIP;
    msgType type;
    Request request; 
    Elevator elev_struct;
	senderRole role;
    bool isEmpty;
} Message;

void sendMessage(Message msg);
Message receiveMessage(void);
bool connectionAvailable(char *ipAddress);
void broadcastIP(void);
void networkInit(void);
char * getMyIP();
