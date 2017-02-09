#include "network_io.h"
#include "../network_driver/sverresnetwork.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_IPS 3

char ips[NUM_IPS][32];

typedef struct Queued_Message QueuedMsg;

struct Queued_Message{
    Message      msg;
    QueuedMsg    *nextMsg;
    //int          value;
};

QueuedMsg *firstMsg;
QueuedMsg *lastMsg;

/*-----------------------------INTERNAL FUNCIONS---------------------------------*/


int add_msg_to_queue (Message newMsg, QueuedMsg *lastMsg, QueuedMsg *firstMsg);
Message get_msg_from_queue (QueuedMsg *firstMsg);
void receiveUDPMsg(const char * ip, char * data, int datalength);
void receiveTCPMsg(const char * ip, char * data, int datalength);
void initIps();
void addIp(const char * ip);
void removeIp(const char * ip);
void tcpConnectionCallback(const char * ip, int created);

int add_msg_to_queue (Message newMsg, QueuedMsg *lastMsg, QueuedMsg *firstMsg){
    QueuedMsg *newQueuedMsg;

    // Create a new action in memory
    if (( newQueuedMsg = (QueuedMsg *)malloc(sizeof(QueuedMsg)) ) == NULL)
        return 0;
    
    // Make the old 'lastAction' point to the new Action, and the new Action to point to NULL
    (*lastMsg).nextMsg = newQueuedMsg;
    newQueuedMsg -> nextMsg = NULL;
    newQueuedMsg -> msg = newMsg;
    
    // Designate the new Action as the new lastAction:
    if (firstMsg == NULL && lastMsg == NULL) {
        firstMsg = newQueuedMsg;
    }
    lastMsg = newQueuedMsg;
    return 1;
}

Message get_msg_from_queue (QueuedMsg *firstMsg){
    Message msg = (*firstMsg).msg;

    //QueuedMsg *secondMsg = (*firstMsg) -> nextMsg;
    //*firstMsg = secondMsg;
    firstMsg = (*firstMsg).nextMsg;
    
    return msg;
}

// typedef void (*TMessageCallback)(const char * ip, char * data, int datalength);
// typedef void (*TTcpConnectionCallback)(const char * ip, int created);

void receiveUDPMsg(const char * ip, char * data, int datalength){
    addIp(ip);
}

void receiveTCPMsg(const char * ip, char * data, int datalength){
    Message msg;
    memcpy( &msg, data, datalength );
    add_msg_to_queue(msg, lastMsg, firstMsg);
}


void initIps(){
    for(int i = 0; i < NUM_IPS; i++){
        ips[i][0] = 0;
    }
}

void addIp(const char * ip){
    int exists = 0;
    for(int i = 0; i < NUM_IPS; i++){
        if(strncmp( ip, ips[i], strlen(ips[i]) ) == 0 && ips[i][0] != 0 ){
	    printf("The ip %s already exists\n",ip);
	    exists = 1;
	}
    }
    if (exists == 0){
    	for(int i = 0; i < NUM_IPS; i++){
            if(ips[i][0] == 0){
                printf("Adding connection %s to index %d\n",ip,i);
                strncpy( ips[i], ip, 30 );
                return;
            }
        }
    }
}

void removeIp(const char * ip){
    for(int i = 0; i < NUM_IPS; i++){
        if(strncmp( ip, ips[i], strlen(ips[i]) ) == 0 && ips[i][0] != 0){
            printf("Removing connection %s from index %d\n",ip,i);
            ips[i][0] = 0;
            return;
        }
    }
}


// created == 1 --> connection created
void tcpConnectionCallback(const char * ip, int created){
    if (created == 1){
	addIp(ip);
    }else{ 
	removeIp(ip);
    }
}


/*-----------------------------EXTERNAL FUNCIONS---------------------------------*/

void sendMessage(Message msg){
    char data[sizeof( Message )];
    memcpy( data, &msg, sizeof( Message ) );
    tcp_send(msg.senderIP, &data[0], sizeof( Message ));
}

// ip of the elevator that sent the message
Message receiveMessage(){
    return get_msg_from_queue(firstMsg);
}

bool connectionAvailable(char *ipAddress){
    for(int i = 0; i < NUM_IPS; ++i) {
        if (strcmp(ips[i], ipAddress) == 0) {
	    return true;
	}
    }
    return false;
}

void broadcastIP(){
    char *iP = 0;
    iP = getMyIpAddress("enp4s0");
    int tcpPortNumber = 5540; // Should be in a config file
    udp_broadcast( tcpPortNumber, iP, strlen(iP) ); // Try with sizeof()
}

void networkInit() {
    initIps();
    tcp_init(receiveTCPMsg, tcpConnectionCallback);
    udp_startReceiving(4040,receiveUDPMsg);
    tcp_startConnectionListening(4040);
}

