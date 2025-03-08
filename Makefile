CC = mpicc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -I/usr/lib/x86_64-linux-gnu/openmpi/include 
LDFLAGS = -lm

SRC = src/image.c src/main.c 
OBJ = $(SRC:.c=.o)
EXEC = tp_kmeans_mpi

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJ) $(LDFLAGS)  # Ajout de $(LDFLAGS)

clean:
	rm -f $(OBJ) $(EXEC)
