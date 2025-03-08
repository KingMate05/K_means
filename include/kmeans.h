#ifndef KMEANS_H
#define KMEANS_H

#include "image.h"

void KMeans_Seq(Image *img, int k, int max_iter);
void KMeans_MPI(Image *img, int k, int max_iter, int rank, int size);

#endif
