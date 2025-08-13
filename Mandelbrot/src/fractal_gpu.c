#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define KERNEL_FILE "fractal.cl"

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

