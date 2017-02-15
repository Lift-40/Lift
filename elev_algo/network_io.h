#include <stdbool.h>
#include "elevator_io_types.h"
#include "elevator.h"

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
    msgType type;
    Request request; 
    Elevator elev_struct;
} Message;

void sendMessage(Message msg);
Message receiveMessage(void);
bool connectionAvailable(char *ipAddress);
void broadcastIP(void);
void networkInit(void);
