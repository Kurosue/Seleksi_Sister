#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "fractal.h"

int main() {
    int width, height, max_iter = 1000;
    double center_x = -0.5, center_y = 0.0, scale = 4.0;

    printf("Width: "); 
    scanf("%d", &width);
    printf("Height: "); 
    scanf("%d", &height);

    unsigned char *image = malloc(width * height * 3);

    printf("\nGenerating (parallel)...\n");
    double start = omp_get_wtime();
    generate_parallel(image, width, height, max_iter, center_x, center_y, scale);
    double end = omp_get_wtime();
    printf("Done in %.3f seconds\n", end - start);

    char filename[256];
    printf("Output filename (without extension): ");
    scanf("%s", filename);
    char path[512];
    snprintf(path, sizeof(path), "image/%s.png", filename);

    if (save_png(path, image, width, height))
        printf("Saved to %s\n", path);
    else
        fprintf(stderr, "Failed to save image\n");

    free(image);
    return 0;
}

