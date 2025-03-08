## K_means
Le projet propose deux versions de l’algorithme K-Means :
- Une version séquentielle (`tp_kmeans_seq`)
- Une version parallèle MPI (`tp_kmeans_mpi`)

### Compilation
make clean
make

### Exécution
./tp_kmeans_seq     #séquentiel
mpirun -np 4 ./tp_kmeans_mpi     #parallèle 

### Visualiser 
gimp images/segmente_seq.ppm    # Résultat séquentiel
gimp images/segmente_mpi.ppm    # Résultat parallèle

### Comparaison
Temps d'exécution (séquentiel) : X.XXX secondes
Temps d'exécution (MPI) : Y.YYY secondes
