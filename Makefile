EXECUTABLE_CLIENT = elev_client
EXECUTABLE_SERVER = elev_server
EXECUTABLE_NETWORK = elev_network

CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu11
LDFLAGS = -lcomedi -lm

CLIENT_SOURCES = elevator.c elevator_io_device.c fsm.c main.c requests.c timer.c driver/io.c
SERVER_SOURCES = elevator_manager.c elevator_storage.c main.c queue.c
NETWORK_SOURCES = network_io.c sverresnetwork.c
CLIENT_OBJECTS = $(addprefix $(CLIENTOBJDIR)/, $(CLIENT_SOURCES:.c=.o))
SERVER_OBJECTS = $(addprefix $(SERVEROBJDIR)/, $(SERVER_SOURCES:.c=.o))
NETWORK_OBJECTS = $(addprefix $(NETWORKOBJDIR)/, $(NETWORK_SOURCES:.c=.o))

CLIENTOBJDIR = clientobj
SERVEROBJDIR = serverobj
NETWORKOBJDIR = networkobj

all: $(EXECUTABLE_NETWORK) $(EXECUTABLE_SERVER) $(EXECUTABLE_CLIENT) 

client: $(EXECUTABLE_NETWORK) $(EXECUTABLE_CLIENT)

server: $(EXECUTABLE_NETWORK) $(EXECUTABLE_SERVER)

rebuild: clean all

clean:
	rm -f $(EXECUTABLE_CLIENT)
	rm -rf $(CLIENTOBJDIR)
	rm -f $(EXECUTABLE_SERVER)
	rm -rf $(SERVEROBJDIR)
	rm -rf $(NETWORKOBJDIR)
	
$(EXECUTABLE_CLIENT): $(CLIENT_OBJECTS)
	$(CC) $^ $(NETWORK_OBJECTS) -o $@ $(LDFLAGS) -pthread
	
$(EXECUTABLE_SERVER): $(SERVER_OBJECTS)
	$(CC) $^ $(NETWORK_OBJECTS) -o $@  $(LDFLAGS) -pthread

$(EXECUTABLE_NETWORK): $(NETWORK_OBJECTS)
	
$(SERVEROBJDIR)/%.o: server/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(CFLAGS) $<
	
$(CLIENTOBJDIR)/%.o: elev_algo/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(CFLAGS) $<

$(NETWORKOBJDIR)/%.o: network_driver/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(CFLAGS) $< -pthread
    
.PHONY: all rebuild clean


