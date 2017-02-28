#include "queue.h"
#include "../drivers/network_io.h"
#include <stdio.h>
#include "../configuration.h"

#include <stdlib.h>
#include <stdio.h>

QueuedReq *firstReq;
QueuedReq *lastReq;

void storeRequest(Request newReq){
	printf("(queue.c)Received request in server: %d, %d\n",newReq.floor,newReq.button);
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
		//printf("Stored request: %d, %d",newReq.floor,newReq.button);
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
		emptyRequest.floor = NUM_FLOORS + 1;
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

QueueArray get_Requests_Array(){
	QueuedReq *firstReq_Copy = firstReq;
	// Get the length/number of requests in the queue
	int length = 0;
	while(firstReq_Copy != NULL){
		firstReq_Copy = (*firstReq_Copy).nextReq;
		length++;
	}
	
	Request *requests_Array;
	requests_Array = (Request *)malloc( sizeof(Request) * length);

	firstReq_Copy = firstReq;
	// Fill requests_Array
	for(int i = 0; i < length; i++){
		requests_Array[i] = (*firstReq_Copy).req;
		firstReq_Copy = (*firstReq_Copy).nextReq;
	}
	QueueArray queueArray;
	queueArray.queue = requests_Array;
	queueArray.length = length;
	return queueArray;
}

