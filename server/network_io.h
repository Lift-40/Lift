#include <stdbool.h>
#include <elevator_io_types.h>
#include <elevator.h>

typedef struct {
    char * senderIP;
    MsgType type;
    Request request; 
    Elevator elev_state;
} Message;

typedef struct {
    int floor;
    Button button; 
} Request;

typedef enum { 
    Request,
    Elev_state,
    light_update
} MsgType;

void sendMessage(Message msg);
Message receiveMessage();
bool connectionAvailable(char *ipAddress);
void broadcastIP();
void addIP(char *ip);
void removeIP(char *ip);
