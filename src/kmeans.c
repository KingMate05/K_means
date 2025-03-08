#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include "../include/image.h"

double Distance(Pixel p1, Pixel p2)
{
    return sqrt(pow(p1.r - p2.r, 2) + pow(p1.g - p2.g, 2) + pow(p1.b - p2.b, 2));
}

void InitialiserCentroïdes(Pixel *centroïdes, Image *img, int k)
{
    srand(42);
    for (int i = 0; i < k; i++)
    {
        int index = rand() % (img->w * img->h);
        centroïdes[i] = img->data[index];
    }
}

int TrouverCentroïdeLePlusProche(Pixel p, Pixel *centroïdes, int k)
{
    int min_index = 0;
    double min_distance = Distance(p, centroïdes[0]);

    for (int i = 1; i < k; i++)
    {
        double d = Distance(p, centroïdes[i]);
        if (d < min_distance)
        {
            min_distance = d;
            min_index = i;
        }
    }
    return min_index;
}

void KMeans_Seq(Image *img, int k, int max_iter)
{
    if (!img || !img->data)
        return;

    int total_pixels = img->w * img->h;
    Pixel centroïdes[k];
    int *labels = (int *)malloc(total_pixels * sizeof(int));
    Pixel *nouveaux_centroïdes = (Pixel *)malloc(k * sizeof(Pixel));
    int *nb_pixels_cluster = (int *)malloc(k * sizeof(int));

    InitialiserCentroïdes(centroïdes, img, k);

    for (int iter = 0; iter < max_iter; iter++)
    {
        for (int i = 0; i < total_pixels; i++)
        {
            labels[i] = TrouverCentroïdeLePlusProche(img->data[i], centroïdes, k);
        }

        for (int i = 0; i < k; i++)
        {
            nouveaux_centroïdes[i].r = 0;
            nouveaux_centroïdes[i].g = 0;
            nouveaux_centroïdes[i].b = 0;
            nb_pixels_cluster[i] = 0;
        }

        for (int i = 0; i < total_pixels; i++)
        {
            int cluster_id = labels[i];
            nouveaux_centroïdes[cluster_id].r += img->data[i].r;
            nouveaux_centroïdes[cluster_id].g += img->data[i].g;
            nouveaux_centroïdes[cluster_id].b += img->data[i].b;
            nb_pixels_cluster[cluster_id]++;
        }

        for (int i = 0; i < k; i++)
        {
            if (nb_pixels_cluster[i] > 0)
            {
                nouveaux_centroïdes[i].r /= nb_pixels_cluster[i];
                nouveaux_centroïdes[i].g /= nb_pixels_cluster[i];
                nouveaux_centroïdes[i].b /= nb_pixels_cluster[i];
            }
            centroïdes[i] = nouveaux_centroïdes[i];
        }
    }

    for (int i = 0; i < total_pixels; i++)
    {
        img->data[i] = centroïdes[labels[i]];
    }

    free(labels);
    free(nouveaux_centroïdes);
    free(nb_pixels_cluster);
}

void KMeans_MPI(Image *img, int k, int max_iter, int rank, int size)
{
    if (!img || !img->data)
    {
        MPI_Abort(MPI_COMM_WORLD, 1);
        return;
    }

    int total_pixels = img->w * img->h;
    int pixels_par_processus = total_pixels / size;
    Pixel centroïdes[k];

    if (rank == 0)
    {
        srand(42);
        for (int i = 0; i < k; i++)
        {
            int index = rand() % total_pixels;
            centroïdes[i] = img->data[index];
        }
    }

    MPI_Bcast(centroïdes, k * sizeof(Pixel), MPI_BYTE, 0, MPI_COMM_WORLD);

    int *labels = (int *)malloc(total_pixels * sizeof(int));
    Pixel *nouveaux_centroïdes = (Pixel *)malloc(k * sizeof(Pixel));
    int *nb_pixels_cluster = (int *)malloc(k * sizeof(int));

    for (int iter = 0; iter < max_iter; iter++)
    {
        for (int i = rank * pixels_par_processus; i < (rank + 1) * pixels_par_processus; i++)
        {
            labels[i] = TrouverCentroïdeLePlusProche(img->data[i], centroïdes, k);
        }

        MPI_Allgather(MPI_IN_PLACE, pixels_par_processus, MPI_INT, labels, pixels_par_processus, MPI_INT, MPI_COMM_WORLD);

        if (rank == 0)
        {
            for (int i = 0; i < k; i++)
            {
                nouveaux_centroïdes[i].r = 0;
                nouveaux_centroïdes[i].g = 0;
                nouveaux_centroïdes[i].b = 0;
                nb_pixels_cluster[i] = 0;
            }

            for (int i = 0; i < total_pixels; i++)
            {
                int cluster_id = labels[i];
                nouveaux_centroïdes[cluster_id].r += img->data[i].r;
                nouveaux_centroïdes[cluster_id].g += img->data[i].g;
                nouveaux_centroïdes[cluster_id].b += img->data[i].b;
                nb_pixels_cluster[cluster_id]++;
            }

            for (int i = 0; i < k; i++)
            {
                if (nb_pixels_cluster[i] > 0)
                {
                    nouveaux_centroïdes[i].r /= nb_pixels_cluster[i];
                    nouveaux_centroïdes[i].g /= nb_pixels_cluster[i];
                    nouveaux_centroïdes[i].b /= nb_pixels_cluster[i];
                }
                centroïdes[i] = nouveaux_centroïdes[i];
            }
        }

        MPI_Bcast(centroïdes, k * sizeof(Pixel), MPI_BYTE, 0, MPI_COMM_WORLD);
    }

    if (rank == 0)
    {
        for (int i = 0; i < total_pixels; i++)
        {
            img->data[i] = centroïdes[labels[i]];
        }
    }

    free(labels);
    free(nouveaux_centroïdes);
    free(nb_pixels_cluster);
}
