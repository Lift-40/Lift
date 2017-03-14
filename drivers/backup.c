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
	    
	fclose(file);
}

void writeElevatorBackup(Elevator* elevator, int id) {
	printf("(backup.c)Attempting store backup for elevator: &d\n", id);
	
	char name[100] = {'\0'};
    sprintf(name, "elevator-%d.bak", id);
    
	FILE *file = fopen(name, "w");
    
	fwrite(elevator, sizeof(Elevator), 1, file);
	
	fclose(file);
}

Server *loadServerBackup() {
	printf("(backup.c)Attempting load server backup\n");
	Server *buf;
	if (( buf = (Server *)malloc(sizeof(Server)) ) == NULL) {
		printf("(backup.c)Out of memory for server!\n");
		return buf;
	}
	buf -> isValid = false;
    FILE *file = fopen(SERVER_BACKUP_PATH, "rb");
	
	if (file != NULL){
        fread(buf, sizeof(Server), 1, file);
		fclose(file);
	} 
	else {
		printf("(backup.c)Failed to load backup!\n");
	}
	return buf;
}

Elevator *loadElevatorBackup(int id) {
	printf("(backup.c)Attempting load backup for elevator: %d\n", id);
	Elevator *buf;
	
	if (( buf = (Elevator *)malloc(sizeof(Elevator)) ) == NULL) {
		printf("(backup.c)Out of memory for elevator!\n");
        exit(1);
	}
	
	char name[100] = {'\0'};
    sprintf(name, "elevator-%d.bak", id);
	
    FILE *file = fopen(name, "rb");
    if (file != NULL){
        fread(buf, sizeof(Elevator), 1, file);
        fclose(file);       
    } 
	else {
		printf("(backup.c)Failed to load elevator backup!\n");
	}
	return buf;
}

void removeElevatorBackup(int id) {
	
	char name[100] = {'\0'};
    sprintf(name, "elevator-%d.bak", id);
    
	fclose(fopen(name, "w"));
}

void removeServerBackup() {
	fclose(fopen(SERVER_BACKUP_PATH, "w"));
}