
#include <omp.h>
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define KERNEL_FILE "fractal.cl"
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



static char* load_kernel_source(const char* filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Kernel file not found");
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    char *source = (char*)malloc(size + 1);
    fread(source, 1, size, fp);
    source[size] = '\0';
    fclose(fp);
    return source;
}

int generate_gpu(unsigned char *image, int width, int height, int max_iter,
                 double center_x, double center_y, double scale, int julia,
                 double c_real, double c_imag) {
    cl_int err;
    cl_uint numPlatforms;
    cl_platform_id platform = NULL;
    cl_device_id device = NULL;

    err = clGetPlatformIDs(1, &platform, &numPlatforms);
    if (err != CL_SUCCESS) { printf("No OpenCL platform found.\n"); return -1; }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (err != CL_SUCCESS) { printf("No GPU device found.\n"); return -1; }

    cl_context context = clCreateContext(0, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);

    char *src = load_kernel_source(KERNEL_FILE);
    if (!src) return -1;

    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&src, NULL, &err);
    free(src);
    if (clBuildProgram(program, 0, NULL, NULL, NULL, NULL) != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char*)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Build error:\n%s\n", log);
        free(log);
        return -1;
    }

    const char *kernel_name = julia ? "julia_kernel" : "mandelbrot_kernel";
    cl_kernel kernel = clCreateKernel(program, kernel_name, &err);

    cl_mem output_buf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, width * height * 3, NULL, NULL);

    // Set arguments
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &output_buf);
    clSetKernelArg(kernel, 1, sizeof(int), &width);
    clSetKernelArg(kernel, 2, sizeof(int), &height);
    clSetKernelArg(kernel, 3, sizeof(double), &center_x);
    clSetKernelArg(kernel, 4, sizeof(double), &center_y);
    clSetKernelArg(kernel, 5, sizeof(double), &scale);
    clSetKernelArg(kernel, 6, sizeof(int), &max_iter);
    if (julia) {
        clSetKernelArg(kernel, 7, sizeof(double), &c_real);
        clSetKernelArg(kernel, 8, sizeof(double), &c_imag);
    }

    size_t global[2] = { (size_t)width, (size_t)height };
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(queue, output_buf, CL_TRUE, 0, width * height * 3, image, 0, NULL, NULL);

    // Cleanup
    clReleaseMemObject(output_buf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return 0;
}

