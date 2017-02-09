#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "network_io.h"

int main(){
    Message testMessage;
    testMessage.senderIP = "127.0.0.1";

    networkInit();

    while(1){
        sleep(4);
        printf("Slept: Going into send loop \n");
        printf("Sending a message to %s\n",testMessage.senderIP);
        sendMessage(testMessage);
    }

    return 0;
}
