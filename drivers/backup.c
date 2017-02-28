#include "../configuration.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../server/elevator_manager.h"
#include "../elevator/elevator.h"
#include "../server/queue.h"

bool isValidElevator(Elevator elevator) {
	return true;
}

bool isValidServer(Server server) {
	return true;
}

void writeServerBackup(Server* server) {
    FILE *file = fopen(SERVER_BACKUP_PATH, "ab");
    fwrite(server, sizeof(Server), 1, file);
    fclose(file);
}

void writeElevatorBackup(Elevator* elevator) {
    FILE *file = fopen(ELEVATOR_BACKUP_PATH, "ab");
    fwrite(elevator, sizeof(Elevator), 1, file);
    fclose(file);
}

Server *loadServerBackup() {
	Server *buf;
	buf = malloc(sizeof(Server));
    FILE *file = fopen(SERVER_BACKUP_PATH, "rb");
    if (file != NULL)
    {
        fread(&buf, sizeof(Server), 1, file);
        fclose(file);       
    }
	return buf;
}

Elevator *loadElevatorBackup() {
	Elevator *buf = malloc(sizeof(Elevator));
    FILE *file = fopen(ELEVATOR_BACKUP_PATH, "rb");
    if (file != NULL)
    {
        fread(&buf, sizeof(Elevator), 1, file);
        fclose(file);       
    }
	return buf;
}

void removeElevatorBackup() {
	fclose(fopen(ELEVATOR_BACKUP_PATH, "w"));
}

void removeServerBackup() {
	fclose(fopen(SERVER_BACKUP_PATH, "w"));
}