#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include "../include/image.h"
#include "../include/kmeans.h"

#define K 20
#define MAX_ITER 15

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Image *img = NULL;
    int img_dims[2] = {0, 0};

    if (rank == 0)
    {
        printf("Processus 0 : Chargement de l’image...\n");
        img = Charger("images/merco.ppm");
        if (!img || !img->data)
        {
            fprintf(stderr, "Processus %d : Erreur lors du chargement de l'image.\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        img_dims[0] = img->w;
        img_dims[1] = img->h;
    }

    MPI_Bcast(img_dims, 2, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Processus %d : Dimensions de l’image reçues (%d x %d)\n", rank, img_dims[0], img_dims[1]);

    if (rank != 0)
    {
        img = NouvelleImage(img_dims[0], img_dims[1]);
        if (!img || !img->data)
        {
            fprintf(stderr, "Processus %d : Erreur d'allocation mémoire pour l'image.\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
    }

    if (img->data == NULL)
    {
        fprintf(stderr, "Processus %d : Erreur, image non allouée avant MPI_Bcast\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
        return 1;
    }

    MPI_Bcast(img->data, img_dims[0] * img_dims[1] * sizeof(Pixel), MPI_BYTE, 0, MPI_COMM_WORLD);
    printf("Processus %d : Pixels de l’image reçus.\n", rank);

    MPI_Barrier(MPI_COMM_WORLD);

    double start, end;
    if (rank == 0)
        start = MPI_Wtime();

    printf("Processus %d : Début de K-Means avec %d clusters et %d itérations.\n", rank, K, MAX_ITER);
    KMeans_MPI(img, K, MAX_ITER, rank, size);

    MPI_Barrier(MPI_COMM_WORLD);
    printf("Processus %d : Fin de K-Means.\n", rank);

    if (rank == 0)
    {
        end = MPI_Wtime();
        printf("Temps d'exécution (MPI) : %f secondes\n", end - start);

        if (Sauver(img, "images/segmente_mpi.ppm"))
        {
            printf("Processus 0 : Image segmentée enregistrée avec succès !\n");
        }
        else
        {
            fprintf(stderr, "Erreur lors de la sauvegarde de l’image segmentée.\n");
        }
        DelImage(img);
    }
    else
    {
        free(img->data);
        free(img);
    }

    MPI_Finalize();
    return 0;
}
