#pragma once

#include "../drivers/network_io.h"

typedef struct QueuedReq QueuedReq;

struct QueuedReq{
    Request      req;
    QueuedReq    *nextReq;
};

typedef struct QueueArray {
	int length;
	Request* queue;
} QueueArray;

void storeRequest(Request newReq);
Request getRequest();
void removeRequest();
QueueArray get_Requests_Array();