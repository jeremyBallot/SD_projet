CC=g++
EXEC=serveur

all: $(EXEC)

serveur: serveur.o
	$(CC) -o serveur serveur.o 

serveur.o: Serveur.c
	$(CC) -o serveur.o -c Serveur.c
clean:
	rm -rf *.o $(EXEC)
