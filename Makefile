CC = mpicc
MPICC = mpicc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lm

SRC_SEQ = src/image.c src/kmeans.c src/main_seq.c
OBJ_SEQ = $(SRC_SEQ:.c=.o)
EXEC_SEQ = tp_kmeans_seq

SRC_MPI = src/image.c src/kmeans.c src/main.c
OBJ_MPI = $(SRC_MPI:.c=.o)
EXEC_MPI = tp_kmeans_mpi

all: $(EXEC_SEQ) $(EXEC_MPI)

$(EXEC_SEQ): $(OBJ_SEQ)
	$(MPICC) $(CFLAGS) -o $(EXEC_SEQ) $(OBJ_SEQ) $(LDFLAGS)

$(EXEC_MPI): $(OBJ_MPI)
	$(MPICC) $(CFLAGS) -o $(EXEC_MPI) $(OBJ_MPI) $(LDFLAGS)

clean:
	rm -f src/*.o $(EXEC_SEQ) $(EXEC_MPI)
