CC:= gcc
CFLAGS:= -Wall -O2 -g
LDFLAGS:=
LIBS:=
OBJS:= singlelink_list.o

.PHONY:clean

singlelink_list: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)    

%*.o: %*.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	-rm *.o singlelink_list
