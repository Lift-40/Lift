#include "network_io.h"
#include "../network_driver/sverresnetwork.h"
#include "../configuration.h"

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
	printf("Allocated memory for new message\n");
	
	newQueuedMsg -> nextMsg = NULL;
	newQueuedMsg -> msg = newMsg;
	
	printf("addMsg: firstMsg: %02x\n", firstMsg);
	
    if (firstMsg == NULL && lastMsg == NULL) {
		printf("Queue is empty, initialized\n");
		firstMsg = newQueuedMsg;
		lastMsg = newQueuedMsg;
		printf("addMsg: firstMsg: %02x\n", firstMsg);
    }else{
		printf("Adding message to queue\n");
		// Make the old 'lastAction' point to the new Action, and the new Action to point to NULL
		(*lastMsg).nextMsg = newQueuedMsg;
		// Designate the new Action as the new lastAction:
    	lastMsg = newQueuedMsg;
	}    
    return 1;
}

Message get_msg_from_queue(){
    if (firstMsg == NULL) {
		Message emptyMessage;
		emptyMessage.isEmpty = true;
        return emptyMessage;
    } else {
		printf("Getting first msg");
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
	printf("New UDP message received\n");
    addIp(ip);
	printf("IP added successfully\n");
    Message msg;
    memcpy( &msg, data, datalength );
	printf("UDP message copied to memory\n");
    add_msg_to_queue(msg);
	printf("UDP message added to queue successfully\n");
}

void receiveTCPMsg(const char * ip, char * data, int datalength){
	printf("New TCP message received\n");
    Message msg;
    memcpy( &msg, data, datalength );
	printf("TCP message copied to memory\n");
    add_msg_to_queue(msg);
	printf("TCP message added to queue successfully\n");
}


void initIps(){
    for(int i = 0; i < MAX_ELEVATORS - 1; i++){
        ips[i][0] = '\0';
    }
}

void addIp(const char * ip){
    int exists = 0;
    for(int i = 0; i < MAX_ELEVATORS - 1; i++){
        if(strcmp( ip, ips[i]) == 0){
			printf("The ip %s already exists\n",ip);
			exists = 1;
		}
    }
    if (exists == 0){
		int added = 0;
    	for(int i = 0; i < MAX_ELEVATORS - 1; i++){
            if(ips[i][0] == '\0' && added == 0){
                printf("Adding connection %s to index %d\n",ip,i);
                strcpy( ips[i], ip);
				added = 1;
            }
        }
    }
}

void removeIp(const char * ip){
    for(int i = 0; i < MAX_ELEVATORS - 1; i++){
        if(strcmp( ip, ips[i]) == 0){
            printf("Removing connection %s from index %d\n",ip,i);
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
	printf("Attempting to send message to %s\n", msg.destinationIP);
	if (!connectionAvailable(msg.destinationIP)) {
		tcp_openConnection(msg.destinationIP,4044);
	}
    char data[sizeof( Message )];
    memcpy( data, &msg, sizeof( Message ) );
    tcp_send(msg.destinationIP, &data[0], sizeof( Message ));
}

// ip of the elevator that sent the message
Message receiveMessage(){
    return get_msg_from_queue();
}

bool connectionAvailable(char *ipAddress){
    for(int i = 0; i < MAX_ELEVATORS - 1; ++i) {
        if (strcmp(ips[i], ipAddress) == 0) {
	    return true;
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
	msg.isEmpty = false;
	printf("msg.senderIP: %s\n", msg.senderIP);
    char data[sizeof( Message )];
    memcpy( data, &msg, sizeof( Message ) );
    udp_broadcast( tcpPortNumber, &data[0], sizeof( Message )); // Try with sizeof()
}

char * getMyIP() {
	printf("My IP address is: %s\n", getMyIpAddress(NETW_INTERFACE));
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
