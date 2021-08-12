CC:= gcc
CFLAGS:= -Wall -O2 -g
LDFLAGS:=
LIBS:=
SER_OBJS:= server.o
CLI_OBJS:= client.o


.PHONY:clean all

all: server client

server: $(SER_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)    

client: $(CLI_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) 
    
%*.o: %*.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	-rm *.o server client 
