#include "../network_driver/network_io.h"

typedef struct Queued_Request QueuedReq;

struct Queued_Request{
    Request      req;
    QueuedReq    *nextReq;
};

void storeRequest(Request newReq);
Request getRequest();
void removeRequest();
QueuedReq *get_Requests_Array();