#include "queue.h"
#include "../network_driver/network_io.h"#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Queued_Request QueuedReq;

struct Queued_Request{
    Request      req;
    QueuedReq    *nextReq;
};

QueuedReq *firstReq;
QueuedReq *lastReq;

void storeRequest(Request newReq){
	printf("Received request: %d, %d\n",newReq.floor,newReq.button);
    QueuedReq *newQueuedReq;

    if (( newQueuedReq = (QueuedReq *)malloc(sizeof(QueuedReq)) ) == NULL) {
        return 0;
	}
	
    newQueuedReq -> nextReq = NULL;
    newQueuedReq -> req = newReq;
    
	if (firstReq == NULL) {
        firstReq = newQueuedReq;
		lastReq = newQueuedReq;
    } else {
		printf("Stored request: %d, %d",newReq.floor,newReq.button);
		(*lastReq).nextReq = newQueuedReq;
		lastReq = newQueuedReq;
	}
    return 1;
}

// This is done in two methods in order to not remove requests till the other side
// has confirm that it has arrived
Request getRequest(){
    if (firstReq == NULL) {
        Request emptyRequest;
		emptyRequest.isEmpty = true;
        return emptyRequest;
    } else {
		Request req = (*firstReq).req;
		return req;
    }
}

void removeRequest(){    
    //QueuedReq *secondReq = (*firstReq) -> nextReq;
    //*firstReq = secondReq;
    firstReq = (*firstReq).nextReq;
}

