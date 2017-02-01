#include "network_io.h"
#include <sverresnetwork.h>

#include <string.h>

#DEFINE NUM_IPS 3

#DEFINE INVALID_IP "INV.ALI.DIP.ADR"

char *iPs[] = {INVALID_IP, INVALID_IP, INVALID_IP};

void sendMessage(Message msg){
    char data[sizeof( struct Message )];
    memcpy( data, &msg, sizeof( struct Message ) );
    tcp_send(msg.senderIP, &data, sizeof( struct Message ));
}

Message receiveMessage(){
    
}

bool connectionAvailable(char *ipAddress){
    for(i = 0; i < NUM_IPS; ++i) {
        if (strcmp(iPs[i], ipAddress) == 0) {
	    return true;
	}
    }
    return false;
}

void broadcastIP(){
    char *iP = INVALID_IP;
    iP = getMyIpAddress(/*"enp4s0"*/);
    int tcpPortNumber = 5540; // Should be in a config file
    udp_broadcast(tcpPortNumber, iP, strlen(iP)); // Try with sizeof()
}

void addIP(char *ip){
    for(i = 0; i < NUM_IPS; ++i) {
        if (strcmp(iPs[i], INVALID_IP) == 0) {
	    iPs[i] = ip;
	    break;
	}
    }
}

void removeIP(char *ip){
    for(i = 0; i < NUM_IPS; ++i) {
        if (strcmp(iPs[i], ip) == 0) {
	    iPs[i] = INVALID_IP;
	    break;
	}
    }
}
