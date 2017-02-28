EXECUTABLE_CLIENT = elev_client
EXECUTABLE_SERVER = elev_server
EXECUTABLE_DRIVERS = elev_drivers

CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu11
LDFLAGS = -lcomedi -lm

CLIENT_SOURCES = elevator.c elevator_io_device.c fsm.c main.c requests.c timer.c driver/io.c
SERVER_SOURCES = elevator_manager.c elevator_storage.c main.c queue.c
DRIVER_SOURCES = network_io.c sverresnetwork.c backup.c
CLIENT_OBJECTS = $(addprefix $(CLIENTOBJDIR)/, $(CLIENT_SOURCES:.c=.o))
SERVER_OBJECTS = $(addprefix $(SERVEROBJDIR)/, $(SERVER_SOURCES:.c=.o))
DRIVER_OBJECTS = $(addprefix $(DRIVEROBJDIR)/, $(DRIVER_SOURCES:.c=.o))

CLIENTOBJDIR = /obj/clientobj
SERVEROBJDIR = /obj/serverobj
DRIVEROBJDIR = /obj/driverobj

all: $(EXECUTABLE_DRIVERS) $(EXECUTABLE_SERVER) $(EXECUTABLE_CLIENT) 

client: $(EXECUTABLE_DRIVERS) $(EXECUTABLE_CLIENT)

server: $(EXECUTABLE_DRIVERS) $(EXECUTABLE_SERVER)

rebuild: clean all

clean:
	rm -f $(EXECUTABLE_CLIENT)
	rm -rf $(CLIENTOBJDIR)
	rm -f $(EXECUTABLE_SERVER)
	rm -rf $(SERVEROBJDIR)
	rm -rf $(DRIVEROBJDIR)
	
$(EXECUTABLE_CLIENT): $(CLIENT_OBJECTS)
	$(CC) $^ $(DRIVER_OBJECTS) -o $@ $(LDFLAGS) -pthread
	
$(EXECUTABLE_SERVER): $(SERVER_OBJECTS)
	$(CC) $^ $(DRIVER_OBJECTS) -o $@  $(LDFLAGS) -pthread

$(EXECUTABLE_DRIVERS): $(DRIVER_OBJECTS)
	
$(SERVEROBJDIR)/%.o: server/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(CFLAGS) $<
	
$(CLIENTOBJDIR)/%.o: elevator/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(CFLAGS) $<

$(DRIVEROBJDIR)/%.o: drivers/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(CFLAGS) $< -pthread
    
.PHONY: all rebuild clean


