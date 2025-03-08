#ifndef IMAGE_H
#define IMAGE_H

#include <mpi.h>

typedef struct
{
    int r, g, b;
} Pixel;

typedef struct
{
    int w, h;
    Pixel *data;
} Image;

Image *NouvelleImage(int w, int h);
Image *Charger(const char *fichier);
int Sauver(Image *img, const char *fichier);
void DelImage(Image *img);

void KMeans_MPI(Image *img, int k, int max_iter, int rank, int size);

#endif
