CC = gcc
all:server_group client c_socket.o
server_group:server_group.c c_socket.o
	$(CC) -g -o $@ $^ -lpthread
client:client.c c_socket.o
	$(CC) -g -o $@ $^ `pkg-config --cflags --libs gtk+-2.0` -lpthread
c_socket.o:c_socket.c
	$(CC) -c $^ -o $@
clean:
	rm server_group client *.o -rf
