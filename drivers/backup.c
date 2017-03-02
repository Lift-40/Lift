#include "../configuration.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../server/elevator_manager.h"
#include "../elevator/elevator.h"
#include "../server/queue.h"

void writeServerBackup(Server* server) {
	printf("(backup.c)Attempting store server backup\n");
    FILE *file = fopen(SERVER_BACKUP_PATH, "w");
    fwrite(server, sizeof(Server), 1, file);
	printf("(backup.c)Backup.isValid: %d\n", server -> isValid);
    fclose(file);
}

void writeElevatorBackup(Elevator* elevator) {
	printf("(backup.c)Attempting store elevator backup\n");
    FILE *file = fopen(ELEVATOR_BACKUP_PATH, "w");
    fwrite(elevator, sizeof(Elevator), 1, file);
    fclose(file);
}

Server *loadServerBackup() {
	printf("(backup.c)Attempting load server backup\n");
	Server *buf;
	if (( buf = (Server *)malloc(sizeof(Server)) ) == NULL) {
		//buf -> isValid = false;
		printf("(backup.c)Out of memory for server!\n");
		return buf;
	}
	buf -> isValid = false;
    FILE *file = fopen(SERVER_BACKUP_PATH, "rb");
	printf("(backup.c)File pointer: %02x\n", file);
    if (file != NULL)
    {
        fread(buf, sizeof(Server), 1, file);
		printf("(backup.c)Backup.isValid: %d\n", buf -> isValid);
        fclose(file);
	} else {
		printf("(backup.c)Failed to load backup!\n");
	}
	return buf;
}

Elevator *loadElevatorBackup() {
	printf("(backup.c)Attempting load elevator backup\n");
	Elevator *buf;
	
	if (( buf = (Elevator *)malloc(sizeof(Elevator)) ) == NULL) {
		printf("(backup.c)Out of memory for elevator!\n");
        exit(1);
	}
	
    FILE *file = fopen(ELEVATOR_BACKUP_PATH, "rb");
    if (file != NULL && access(ELEVATOR_BACKUP_PATH, R_OK))
    {
        fread(buf, sizeof(Elevator), 1, file);
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