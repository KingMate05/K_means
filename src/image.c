#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
