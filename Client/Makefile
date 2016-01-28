CC=g++
EXEC=client

all: $(EXEC)

client: client.o
	$(CC) -o client client.o 

client.o: Client.c
	$(CC) -o client.o -c Client.c
clean:
	rm -rf *.o $(EXEC)
