#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/image.h"
#include "../include/kmeans.h"

#define K 20
#define MAX_ITER 15

int main()
{
    Image *img = Charger("images/merco.ppm");
    if (!img || !img->data)
    {
        fprintf(stderr, "Erreur lors du chargement de l'image.\n");
        return 1;
    }

    clock_t start = clock();
    KMeans_Seq(img, K, MAX_ITER);
    double time_spent = (double)(clock() - start) / CLOCKS_PER_SEC;

    printf("Temps d'exécution (séquentiel) : %f secondes\n", time_spent);

    if (Sauver(img, "images/segmente_seq.ppm"))
    {
        printf("Image segmentée (séquentielle) enregistrée avec succès !\n");
    }
    else
    {
        fprintf(stderr, "Erreur lors de la sauvegarde de l'image segmentée.\n");
    }

    DelImage(img);
    return 0;
}
