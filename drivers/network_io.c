#include "network_io.h"
#include "../drivers/sverresnetwork.h"
#include "../configuration.h"
#include "../elevator/elevator.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char ids[MAX_ELEVATORS];

typedef struct Queued_Message QueuedMsg;

struct Queued_Message{
    Message      msg;
    QueuedMsg    *nextMsg;
};

#define NETW_INTERFACE "lo"

QueuedMsg *firstMsg;
QueuedMsg *lastMsg;

/*-----------------------------INTERNAL FUNCTIONS---------------------------------*/


int add_msg_to_queue (Message newMsg);
Message get_msg_from_queue ();
void receiveUDPMsg(const char * ip, char * data, int datalength);
void receiveTCPMsg(const char * ip, char * data, int datalength);
void initIds();
void addId(int id);
void removeId(int id);
void tcpConnectionCallback(int id, int created);

int add_msg_to_queue (Message newMsg){
    QueuedMsg *newQueuedMsg;

    // Create a new action in memory
    if (( newQueuedMsg = (QueuedMsg *)malloc(sizeof(QueuedMsg)) ) == NULL) {
        return 0;
	}
	
	newQueuedMsg -> nextMsg = NULL;
	newQueuedMsg -> msg = newMsg;
	
	if (firstMsg == NULL) {
		printf("(network_io.c)Queue is empty, initialized\n");
		firstMsg = newQueuedMsg;
		lastMsg = newQueuedMsg;
		printf("(network_io.c)Adding first message: %02x\n", firstMsg);
    }else{
		// Make the old 'lastAction' point to the new Action, and the new Action to point to NULL
		(*lastMsg).nextMsg = newQueuedMsg;
		// Designate the new Action as the new lastAction:
    	lastMsg = newQueuedMsg;
		printf("(network_io.c)Adding message to the end of the queue\n");
	}    
    return 1;
}

Message get_msg_from_queue(){
    if (firstMsg == NULL) {
		Message emptyMessage;
		emptyMessage.isEmpty = true;
        return emptyMessage;
    } else {
		printf("(network_io.c)Getting first message\n");
		QueuedMsg *prevMsg = firstMsg;
        firstMsg = (*firstMsg).nextMsg;
		return (*prevMsg).msg;
    }
}

void receiveUDPMsg(const char * ip, char * data, int datalength){
	printf("\n(network_io.c)New UDP message received\n\n");
	
    Message msg;
	memcpy( &msg, data, sizeof(Message) );
	
	add_msg_to_queue(msg);
}

void receiveTCPMsg(const char * ip, char * data, int datalength){
	printf("\n(network_io.c)New TCP message received\n\n");
	
    Message msg;
	memcpy( &msg, data, sizeof(Message) );
	
    add_msg_to_queue(msg);
}


void initIds(){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        ids[i] = -1;
    }
}

void addId(int id){
    int exists = 0;
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if( id == ids[i] ){
			printf("(network_io.c)The id %d of an elevator already exists\n", id);
			exists = 1;
		}
    }
    if (exists == 0){
		int added = 0;
    	for(int i = 0; i < MAX_ELEVATORS; i++){
            if(ids[i] == -1 && added == 0){
                printf("(network_io.c)Adding connection with elevator %d to index %d\n", id, i);
                ids[i] = id;
				added = 1;
            }
        }
    }
}

void removeId(int id){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if( id == ids[i] ){
            printf("(network_io.c)Removing connection with elevator %d from index %d\n", id, i);
            ids[i] = -1;
        }
    }
}

// created == 1 --> connection created
void tcpConnectionCallback(int id, int created){
	if (created == 1){
		addId(id);
	} else { 
		removeId(id);
	}
}

/*-----------------------------EXTERNAL FUNCTIONS---------------------------------*/

void sendMessage(Message msg){
	int tcpPortNumber;
	int id;
	
	if (msg.role == server){	
		printf("(network_io.c)Attempting to send message to elevator with IP: %s\n", msg.destinationIP);
		tcpPortNumber = BASE_PORT + 10*msg.elevatorID+3;
		id = msg.elevatorID;
	} 
	else if (msg.role == elev){
		printf("(network_io.c)Attempting to send message to server with IP: %s\n", msg.destinationIP);
		tcpPortNumber = BASE_PORT + 10*msg.elevatorID+1;
		id = 0;
	}

	if (!connectionAvailable( id )) {
		tcp_openConnection(msg.destinationIP, tcpPortNumber, id);
	}
	
    char data[ sizeof(Message) ];
	memcpy( data, &msg, sizeof( Message ) );
	
	if (msg.role == server){
		tcp_send(msg.destinationIP, &data, sizeof( Message ), id);
	}
	else if (msg.role == elev){
		tcp_send(msg.destinationIP, &data, sizeof( Message ), id);
	}
}

void printMsg(Message msg){
	printf("(network_io.c)Sender IP: %s\n", msg.senderIP);
	printf("(network_io.c)Destination IP: %s\n", msg.destinationIP);
	printf("(network_io.c)Message type: ");
	if (msg.type == req) {
		printf("req\n");
		printReq(msg.request);
	} else if (msg.type == elev_state) {
		printf("elev_state\n");
	} else if (msg.type == light_update) {
		printf("light_update\n");
	} else if (msg.type == broadcast) {
		printf("broadcast\n");
	} else {
		printf("Invalid type\n");
	}
    
	if(msg.isEmpty) {
		printf("(network_io.c)Message.isEmpty: true\n");
	} else if (!msg.isEmpty) {
		printf("(network_io.c)Message.isEmpty: false\n");
	} else {
		printf("(network_io.c)Message.isEmpty: Invalid\n");
	}
	printf("\n");
}

void printReq(Request req) {
	printf("\n(network_io.c)Printing request %02x\n", req);
	printf("(network_io.c)Floor: %d\n", req.floor);
	printf("(network_io.c)Button type: %02x\n", req.button);
}

Message receiveMessage(){
    return get_msg_from_queue();
}

bool connectionAvailable(int id){
	if (id != -1){
		for(int i = 0; i < MAX_ELEVATORS; i++) {
			if (ids[i] == id) {
				return true;
			}
		}
	}
    return false;
}

void broadcastIP(senderRole role, int elevatorID){
	int udpPortNumber;
	Message msg;
	strcpy(msg.senderIP, getMyIP());
	msg.role = role;
	msg.type = broadcast;
	msg.elevatorID = elevatorID;
	msg.isEmpty = false;
	
	printf("(network_io.c)Broadcasting IP, msg.senderIP: %s\n", msg.senderIP);
    
	char data[sizeof( Message )];
    memcpy( data, &msg, sizeof( Message ) );
	
	if (role == server) {
		for(int i = 1; i < MAX_ELEVATORS + 1; i++) {
			udpPortNumber = BASE_PORT + 10*i+2;
    		udp_broadcast( udpPortNumber, &data[0], sizeof( Message ));
		}
	}
	else {
		udpPortNumber = BASE_PORT;
		udp_broadcast( udpPortNumber, &data[0], sizeof( Message ));
	}
}

char * getMyIP() {
	printf("(network_io.c)My IP address is: %s\n", getMyIpAddress(NETW_INTERFACE));
	return getMyIpAddress(NETW_INTERFACE);
}

void networkInit(int elevatorID, senderRole role) {
    initIds();
    tcp_init(receiveTCPMsg, tcpConnectionCallback);
	
	if (role == server) {
		udp_startReceiving(BASE_PORT, receiveUDPMsg);
		for(int i = 1; i < MAX_ELEVATORS + 1; i++) {
			tcp_startConnectionListening( i+3 );
			sleep(1);
		}
	} 
	else {
		udp_startReceiving(BASE_PORT + 10*elevatorID+2,receiveUDPMsg);
		tcp_startConnectionListening( elevatorID );
	}
}