#ifndef MANDELBROT_H
#define MANDELBROT_H

#ifdef __cplusplus
extern "C" {
#endif

void generate_serial(unsigned char *image, int width, int height,
                     int max_iter, double center_x, double center_y, double scale);
void generate_parallel(unsigned char *image, int width, int height,
                       int max_iter, double center_x, double center_y, double scale);

int save_png(const char *path, const unsigned char *image, int width, int height);

void generate_julia_serial(unsigned char *img, int width, int height,
        int max_iter, double center_x, double center_y, double scale,
        double c_real, double c_imag);
void generate_julia_parallel(unsigned char *img, int width, int height,
        int max_iter, double center_x, double center_y, double scale,
        double c_real, double c_imag);

int generate_gpu(unsigned char *image, int width, int height, int max_iter,
                 double center_x, double center_y, double scale, int julia,
                 double c_real, double c_imag);

#ifdef __cplusplus
}
#endif

#endif

