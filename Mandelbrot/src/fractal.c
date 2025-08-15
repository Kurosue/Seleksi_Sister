
#include <omp.h>
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include "fractal.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static inline void iter_to_rgb(int iter, int max_iter, unsigned char *r, unsigned char *g, unsigned char *b) {
    if (iter >= max_iter) {
        *r = *g = *b = 0;
        return;
    }
    double t = (double)iter / (double)max_iter;
    int ir = (int)(9*(1-t)*t*t*t*255);
    int ig = (int)(15*(1-t)*(1-t)*t*t*255);
    int ib = (int)(8.5*(1-t)*(1-t)*(1-t)*t*255);
    *r = (unsigned char)(ir & 0xFF);
    *g = (unsigned char)(ig & 0xFF);
    *b = (unsigned char)(ib & 0xFF);
}

void generate_serial(unsigned char *image, int width, int height,
                     int max_iter, double center_x, double center_y, double scale) {
    double aspect_ratio = (double)width / height;
    double x_min = center_x - scale / 2;
    double x_max = center_x + scale / 2;
    double y_min = center_y - (scale / aspect_ratio) / 2;
    double y_max = center_y + (scale / aspect_ratio) / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double zx = 0.0, zy = 0.0;
            double cX = x_min + (x / (double)width) * (x_max - x_min);
            double cY = y_min + (y / (double)height) * (y_max - y_min);
            int iter;
            for (iter = 0; iter < max_iter; iter++) {
                double tmp = zx * zx - zy * zy + cX;
                zy = 2.0 * zx * zy + cY;
                zx = tmp;
                if ((zx * zx + zy * zy) > 4.0) break;
            }
            unsigned char r, g, b;
            iter_to_rgb(iter, max_iter, &r, &g, &b);
            int idx = (y * width + x) * 3;
            image[idx] = r;
            image[idx+1] = g;
            image[idx+2] = b;
        }
    }
}

void generate_parallel(unsigned char *image, int width, int height,
                       int max_iter, double center_x, double center_y, double scale) {
    double aspect_ratio = (double)width / height;
    double x_min = center_x - scale / 2;
    double x_max = center_x + scale / 2;
    double y_min = center_y - (scale / aspect_ratio) / 2;
    double y_max = center_y + (scale / aspect_ratio) / 2;

    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double zx = 0.0, zy = 0.0;
            double cX = x_min + (x / (double)width) * (x_max - x_min);
            double cY = y_min + (y / (double)height) * (y_max - y_min);
            int iter;
            for (iter = 0; iter < max_iter; iter++) {
                double tmp = zx * zx - zy * zy + cX;
                zy = 2.0 * zx * zy + cY;
                zx = tmp;
                if ((zx * zx + zy * zy) > 4.0) break;
            }
            unsigned char r, g, b;
            iter_to_rgb(iter, max_iter, &r, &g, &b);
            int idx = (y * width + x) * 3;
            image[idx] = r;
            image[idx+1] = g;
            image[idx+2] = b;
        }
    }
}

int save_png(const char *path, const unsigned char *image, int width, int height) {
    return stbi_write_png(path, width, height, 3, image, width * 3) != 0;
}

static int julia_pixel(double zx, double zy, double c_real, double c_imag, int max_iter) {
    int iter = 0;
    while (zx*zx + zy*zy < 4.0 && iter < max_iter) {
        double tmp = zx*zx - zy*zy + c_real;
        zy = 2.0*zx*zy + c_imag;
        zx = tmp;
        iter++;
    }
    return iter;
}

void generate_julia_serial(unsigned char *img, int width, int height,
    int max_iter, double center_x, double center_y, double scale,
    double c_real, double c_imag)
{
    double aspect = (double)width / (double)height;
    double x_min = center_x - scale/2.0;
    double y_min = center_y - (scale/aspect)/2.0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double zx = x_min + (double)x / width * scale;
            double zy = y_min + (double)y / height * (scale/aspect);
            int iter = julia_pixel(zx, zy, c_real, c_imag, max_iter);
            int idx = (y * width + x) * 3;
            unsigned char color = (unsigned char)(255.0 * iter / max_iter);
            img[idx] = color;
            img[idx+1] = color;
            img[idx+2] = color;
        }
    }
}

void generate_julia_parallel(unsigned char *img, int width, int height,
    int max_iter, double center_x, double center_y, double scale,
    double c_real, double c_imag)
{
    double aspect = (double)width / (double)height;
    double x_min = center_x - scale/2.0;
    double y_min = center_y - (scale/aspect)/2.0;

    #pragma omp parallel for schedule(dynamic)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double zx = x_min + (double)x / width * scale;
            double zy = y_min + (double)y / height * (scale/aspect);
            int iter = julia_pixel(zx, zy, c_real, c_imag, max_iter);
            int idx = (y * width + x) * 3;
            unsigned char color = (unsigned char)(255.0 * iter / max_iter);
            img[idx] = color;
            img[idx+1] = color;
            img[idx+2] = color;
        }
    }
}



