#include "../configuration.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../server/elevator_manager.h"
#include "../elevator/elevator.h"
#include "../server/queue.h"

void writeServerBackup(Server* server) {
    FILE *file = fopen(SERVER_BACKUP_PATH, "w");
    fwrite(server, sizeof(Server), 1, file);
    fclose(file);
}

void writeElevatorBackup(Elevator* elevator) {
    FILE *file = fopen(ELEVATOR_BACKUP_PATH, "w");
    fwrite(elevator, sizeof(Elevator), 1, file);
    fclose(file);
}

Server *loadServerBackup() {
	Server *buf;
	
	if (( buf = (Server *)malloc(sizeof(Server)) ) == NULL) {
		printf("(backup.c)Out of memory for server!\n");
        exit(1);
	}
	
	buf -> isValid = false;
    FILE *file = fopen(SERVER_BACKUP_PATH, "rb");
    if (file != NULL && access(SERVER_BACKUP_PATH, R_OK))
    {
        fread(&buf, sizeof(Server), 1, file);
        fclose(file);
    }
	return buf;
}

Elevator *loadElevatorBackup() {
	Elevator *buf;
	
	if (( buf = (Elevator *)malloc(sizeof(Elevator)) ) == NULL) {
		printf("(backup.c)Out of memory for elevator!\n");
        exit(1);
	}
	
    FILE *file = fopen(ELEVATOR_BACKUP_PATH, "rb");
    if (file != NULL && access(ELEVATOR_BACKUP_PATH, R_OK))
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