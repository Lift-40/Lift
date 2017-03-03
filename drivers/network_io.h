#pragma once
#include <stdbool.h>
#include "../elevator/elevator_io_types.h"
#include "../elevator/elevator.h"

typedef struct {
    int floor;
    Button button;
	bool isEmpty;
} Request;

typedef enum { 
    server,
	elev
} senderRole;

typedef enum { 
    req,
    elev_state,
    light_update,
	broadcast
} msgType;

typedef struct {
    char senderIP[32];
    char destinationIP[32];
	int elevatorID;
    msgType type;
    Request request; 
    Elevator elev_struct;
    senderRole role;
    bool isEmpty;
} Message;

void sendMessage(Message msg);
Message receiveMessage(void);
bool connectionAvailable(char *ipAddress);
void broadcastIP(senderRole role, int elevatorID);
void networkInit(int port_Number, senderRole role);
void printReq(Request req);
char * getMyIP();
void printMessage(Message msg);
