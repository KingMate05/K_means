#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include "../include/image.h"

Image *NouvelleImage(int w, int h)
{
    Image *img = (Image *)malloc(sizeof(Image));
    if (!img)
    {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour l'image\n");
        exit(EXIT_FAILURE);
    }
    img->w = w;
    img->h = h;
    img->data = (Pixel *)malloc(w * h * sizeof(Pixel));
    if (!img->data)
    {
        fprintf(stderr, "Erreur : Allocation mémoire échouée pour les pixels\n");
        free(img);
        exit(EXIT_FAILURE);
    }
    return img;
}

Image *Charger(const char *fichier)
{
    printf("Tentative d'ouverture du fichier : %s\n", fichier);

    FILE *fp = fopen(fichier, "r");
    if (!fp)
    {
        perror("Erreur ouverture fichier");
        return NULL;
    }

    char format[3];
    fscanf(fp, "%2s", format);
    if (format[0] != 'P' || (format[1] != '3' && format[1] != '6'))
    {
        fprintf(stderr, "Format non supporté\n");
        fclose(fp);
        return NULL;
    }

    int w, h, max;
    fscanf(fp, "%d %d %d", &w, &h, &max);

    Image *img = NouvelleImage(w, h);
    if (!img || !img->data)
    {
        fprintf(stderr, "Erreur allocation mémoire pour l'image\n");
        fclose(fp);
        return NULL;
    }

    if (format[1] == '3')
    {
        for (int i = 0; i < w * h; i++)
        {
            if (fscanf(fp, "%d %d %d", &img->data[i].r, &img->data[i].g, &img->data[i].b) != 3)
            {
                fprintf(stderr, "Erreur de lecture des pixels\n");
                free(img->data);
                free(img);
                fclose(fp);
                return NULL;
            }
        }
    }
    else
    {
        fgetc(fp);
        fread(img->data, sizeof(Pixel), w * h, fp);
    }

    fclose(fp);
    return img;
}

int Sauver(Image *img, const char *fichier)
{
    FILE *fp = fopen(fichier, "w");
    if (!fp)
    {
        fprintf(stderr, "Erreur : Impossible d'ouvrir le fichier %s pour l'écriture\n", fichier);
        return 0;
    }

    fprintf(fp, "P3\n%d %d\n255\n", img->w, img->h);
    for (int i = 0; i < img->w * img->h; i++)
    {
        fprintf(fp, "%d %d %d\n", img->data[i].r, img->data[i].g, img->data[i].b);
    }

    fclose(fp);
    return 1;
}

void DelImage(Image *img)
{
    if (img != NULL)
    {
        if (img->data != NULL)
        {
            free(img->data);
            img->data = NULL;
        }
        free(img);
    }
}

double Distance(Pixel p1, Pixel p2)
{
    return sqrt(pow(p1.r - p2.r, 2) + pow(p1.g - p2.g, 2) + pow(p1.b - p2.b, 2));
}

void InitialiserCentroïdes(Pixel *centroïdes, Image *img, int k)
{
    srand(time(NULL));
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

void KMeans_MPI(Image *img, int k, int max_iter, int rank, int size)
{
    if (!img || !img->data)
    {
        fprintf(stderr, "Processus %d : Image non allouée, arrêt de K-Means.\n", rank);
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
