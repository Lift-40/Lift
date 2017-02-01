#include "network_io.h"
#include <sverresnetwork.h>

void sendMessage(Message msg){
    char data[sizeof( struct Message )];
    memcpy( data, &msg, sizeof( struct Message ) );
    tcp_send(msg.senderIP, &data, sizeof( struct Message ));
}

Message receiveMessage(){
    
}

