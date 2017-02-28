#include "../elevator/elevator.h"
#include "../server/elevator_manager.h"

bool isValidElevator(Elevator* elevator);
bool isValidServer(Server* server);
void writeServerBackup(Server* server);
void writeElevatorBackup(Elevator* elevator);
Server loadServerBackup();
Elevator loadElevatorBackup();
void removeElevatorBackup();
void removeServerBackup();