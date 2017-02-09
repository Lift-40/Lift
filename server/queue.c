#include "queue.h"
#include "network_io.h"

typedef struct Queued_Request QueuedReq;

struct Queued_Request{
    Request      req;
    QueuedReq    *nextReq;
};

QueuedReq *firstReq;
QueuedReq *lastReq;

void storeRequest(Request newReq){
    QueuedReq *newQueuedReq;

    if (( newQueuedReq = (QueuedReq *)malloc(sizeof(QueuedReq)) ) == NULL)
        return 0;
    
    (*lastReq).nextReq = newQueuedReq;
    newQueuedReq -> nextReq = NULL;
    newQueuedReq -> req = newReq;
    
    if (firstReq == NULL && lastReq == NULL) {
        firstReq = newQueuedReq;
    }

    lastReq = newQueuedReq;
    return 1;
}

// This is done in two methods in order to not remove requests till the other side
// has confirm that it has arrived
Request getRequest(){
    if (firstReq == NULL) {
        return NULL;
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

