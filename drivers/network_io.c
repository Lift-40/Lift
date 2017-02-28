#include "network_io.h"
#include "../drivers/sverresnetwork.h"
#include "../configuration.h"
#include "../elevator/elevator.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char ips[MAX_ELEVATORS][32];

typedef struct Queued_Message QueuedMsg;

struct Queued_Message{
    Message      msg;
    QueuedMsg    *nextMsg;
    //int          value;
};

#define NETW_INTERFACE "lo"

QueuedMsg *firstMsg;
QueuedMsg *lastMsg;

/*-----------------------------INTERNAL FUNCIONS---------------------------------*/


int add_msg_to_queue (Message newMsg);
Message get_msg_from_queue ();
void receiveUDPMsg(const char * ip, char * data, int datalength);
void receiveTCPMsg(const char * ip, char * data, int datalength);
void initIps();
void addIp(const char * ip);
void removeIp(const char * ip);
void tcpConnectionCallback(const char * ip, int created);

int add_msg_to_queue (Message newMsg){
    QueuedMsg *newQueuedMsg;

    // Create a new action in memory
    if (( newQueuedMsg = (QueuedMsg *)malloc(sizeof(QueuedMsg)) ) == NULL) {
        return 0;
	}
	//printf("Allocated memory for new message\n");
	
	newQueuedMsg -> nextMsg = NULL;
	newQueuedMsg -> msg = newMsg;
	
	//printf("addMsg: first message in queue: %02x\n", firstMsg);
	
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
		//printf("(network_io.c)There is no message in the queue\n");
		Message emptyMessage;
		emptyMessage.isEmpty = true;
        return emptyMessage;
    } else {
		printf("(network_io.c)Getting first message\n");
        //QueuedMsg *secondMsg = (*firstMsg) -> nextMsg;
        //*firstMsg = secondMsg;
		QueuedMsg *prevMsg = firstMsg;
        firstMsg = (*firstMsg).nextMsg;
		return (*prevMsg).msg;
    }
}

// typedef void (*TMessageCallback)(const char * ip, char * data, int datalength);
// typedef void (*TTcpConnectionCallback)(const char * ip, int created);

void receiveUDPMsg(const char * ip, char * data, int datalength){
	printf("\n(network_io.c)New UDP message received\n\n");
	//printf("IP added successfully\n");
	//data = (char *)&Message;
    Message msg;
	//data = (char *)&msg;
    memcpy( &msg, data, sizeof(Message) );
	//printf("UDP message copied to memory\n");
	printMsg(msg);
    add_msg_to_queue(msg);
	addIp(msg.senderIP);
	//printf("UDP message added to queue successfully\n");
}

void receiveTCPMsg(const char * ip, char * data, int datalength){
	printf("\n(network_io.c)New TCP message received\n\n");
	//printf("Datalength: %d", datalength);
	//data = (char *)&Message;
    Message msg;
	//data = (char *)&msg;
    memcpy( &msg, data, sizeof(Message) );
	//printf("TCP message copied to memory\n");
	printMsg(msg);
    add_msg_to_queue(msg);
	//printf("TCP message added to queue successfully\n");
}


void initIps(){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        ips[i][0] = '\0';
    }
}

void addIp(const char * ip){
    int exists = 0;
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(strcmp( ip, ips[i]) == 0){
			printf("(network_io.c)The ip %s of an elevator already exists\n",ip);
			exists = 1;
		}
    }
    if (exists == 0){
		int added = 0;
    	for(int i = 0; i < MAX_ELEVATORS; i++){
            if(ips[i][0] == '\0' && added == 0){
                printf("(network_io.c)Adding connection with elevator %s to index %d\n",ip,i);
                strcpy( ips[i], ip);
				added = 1;
            }
        }
    }
}

void removeIp(const char * ip){
    for(int i = 0; i < MAX_ELEVATORS; i++){
        if(strcmp( ip, ips[i]) == 0){
            printf("(network_io.c)Removing connection with elevator %s from index %d\n",ip,i);
            ips[i][0] = '\0';
        }
    }
}


// created == 1 --> connection created
void tcpConnectionCallback(const char * ip, int created){
	if (ip != 0) {
		if (created == 1){
			addIp(ip);
		} else { 
			removeIp(ip);
		}
	}
}


/*-----------------------------EXTERNAL FUNCIONS---------------------------------*/

void sendMessage(Message msg){
	int tcpPortNumber;
	if (msg.role == server){
		printf("(network_io.c)Attempting to send message to elevator with IP: %s\n", msg.destinationIP);
		tcpPortNumber = 4043;
	} else if (msg.role == elev){
		printf("(network_io.c)Attempting to send message to server with IP: %s\n", msg.destinationIP);
		tcpPortNumber = 4044;
	}

	printf("(network_io.c)connectionAvailable(msg.destinationIP): %i\n", connectionAvailable(msg.destinationIP));
	if (!connectionAvailable(msg.destinationIP)) {
		tcp_openConnection(msg.destinationIP,tcpPortNumber);
	}
	
    char data[ sizeof(Message) ];
	
	//printf("message size: %d\n", sizeof(Message));
    memcpy( data, &msg, sizeof( Message ) );
	//printf("memcpy data: %s\n", data);
    tcp_send(msg.destinationIP, &data, sizeof( Message ));
    Message sentmsg;
    memcpy( &sentmsg, data, sizeof(Message) );
	printf("(network_io.c)Message that arrives\n");
	printMsg(msg);
	printf("(network_io.c)Message that is sent\n");
	printMsg(sentmsg);
}

void printMsg(Message msg){
	//printf("\nPrinting message %02x\n", msg);
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
    //elevator_print(msg.elev_struct);
    //senderRole role;
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

// ip of the elevator that sent the message
Message receiveMessage(){
    return get_msg_from_queue();
}

bool connectionAvailable(char *ipAddress){
	if (ipAddress[0] != 0) {
		for(int i = 0; i < MAX_ELEVATORS; i++) {
			if (strcmp(ips[i], ipAddress) == 0) {
				return true;
			}
		}
	}
    return false;
}

void broadcastIP(senderRole role){
	int tcpPortNumber;
	if (role == server) {
		tcpPortNumber = 4040;
	} else {
		tcpPortNumber = 4041;
	}
	Message msg;
	strcpy(msg.senderIP, getMyIP());
	// msg.senderIP = getMyIP();
	msg.role = role;
	msg.type = broadcast;
	msg.isEmpty = false;
	printf("(network_io.c)Broadcasting IP, msg.senderIP: %s\n", msg.senderIP);
    char data[sizeof( Message )];
    memcpy( data, &msg, sizeof( Message ) );
    udp_broadcast( tcpPortNumber, &data[0], sizeof( Message )); // Try with sizeof()
}

char * getMyIP() {
	printf("(network_io.c)My IP address is: %s\n", getMyIpAddress(NETW_INTERFACE));
	return getMyIpAddress(NETW_INTERFACE);
}

void networkInit(int port_Number, senderRole role) {
    initIps();
    tcp_init(receiveTCPMsg, tcpConnectionCallback);
	if (role == server) {
		udp_startReceiving(4041,receiveUDPMsg);
		tcp_startConnectionListening(4044);
	} else {
		udp_startReceiving(4040,receiveUDPMsg);
		tcp_startConnectionListening(port_Number);
	}
}
