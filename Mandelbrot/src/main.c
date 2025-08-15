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
    double time_serial, time_parallel, speedup;

    printf("\nGenerating (serial)...\n");
    double start_serial = omp_get_wtime();
    generate_serial(image, width, height, max_iter, center_x, center_y, scale);
    double end_serial = omp_get_wtime();
    time_serial = end_serial - start_serial;
    printf("Serial done in %.3f seconds\n", time_serial);

    printf("\nGenerating (parallel)...\n");
    double start_parallel = omp_get_wtime();
    generate_parallel(image, width, height, max_iter, center_x, center_y, scale);
    double end_parallel = omp_get_wtime();
    time_parallel = end_parallel - start_parallel;
    printf("Parallel done in %.3f seconds\n", time_parallel);

    speedup = time_serial / time_parallel;
    printf("\n=== BENCHMARK RESULTS ===\n");
    printf("Serial time:   %.3f seconds\n", time_serial);
    printf("Parallel time: %.3f seconds\n", time_parallel);
    printf("Speedup:       %.2fx (parallel is %.2fx faster)\n", speedup, speedup);

    char filename[256];
    printf("\nOutput filename (without extension): ");
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

