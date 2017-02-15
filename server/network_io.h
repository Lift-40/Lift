#include <stdbool.h>
#include "../elev_algo/elevator_io_types.h"
#include "../elev_algo/elevator.h"

typedef struct {
    int floor;
    Button button; 
} Request;

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
} Message;

void sendMessage(Message msg);
Message receiveMessage(void);
bool connectionAvailable(char *ipAddress);
void broadcastIP(void);
void networkInit(void);
char * getMyIP();
