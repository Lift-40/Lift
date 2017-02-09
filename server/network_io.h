#include <stdbool.h>
#include "../elev_algo/elevator_io_types.h"
#include "../elev_algo/elevator.h"

typedef struct {
    int floor;
    Button button; 
} Request;

typedef enum { 
    Req,
    Elev_state,
    light_update
} MsgType;

typedef struct {
    char * senderIP;
    MsgType type;
    Request request; 
    Elevator elev_state;
} Message;

void sendMessage(Message msg);
Message receiveMessage(void);
bool connectionAvailable(char *ipAddress);
void broadcastIP(void);
void networkInit(void);
