#include "../server/elevator_manager.h"

void writeServerBackup(Server* server);
void writeElevatorBackup(Elevator* elevator, int id);
Server *loadServerBackup();
Elevator *loadElevatorBackup(int id);
void removeElevatorBackup(int id);
void removeServerBackup();