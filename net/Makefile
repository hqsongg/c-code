CC:= gcc
CFLAGS:= -Wall -O2 -g
LDFLAGS:=
LIBS:=
SER_OBJS:= server.o
CLI_OBJS:= client.o
TARGETS:= server client pipe cgi-server server-select


.PHONY:clean all 

all: $(TARGETS)

server: $(SER_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)    

client: $(CLI_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) 

%*.o: %*.c
	$(CC) -o $@ $^ $(CFLAGS)
    
    
pipe: pipe-test.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) 
    
cgi-server: dup-cgi-server.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) 
    
server-select: server-select.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) 

clean:
	-rm -rf *.o $(TARGETS) 
