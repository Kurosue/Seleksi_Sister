__kernel void mandelbrot_kernel(
    __global uchar *output,
    const int width,
    const int height,
    const double center_x,
    const double center_y,
    const double scale,
    const int max_iter
) {
    int gx = get_global_id(0);
    int gy = get_global_id(1);
    if (gx >= width || gy >= height) return;

    double x0 = center_x + (gx - width / 2.0) * scale / width;
    double y0 = center_y + (gy - height / 2.0) * scale / width;

    double x = 0.0;
    double y = 0.0;
    int iter = 0;
    while (x*x + y*y <= 4.0 && iter < max_iter) {
        double xtemp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = xtemp;
        iter++;
    }

    float t = (float)iter / (float)max_iter;
    uchar r = (uchar)(9*(1-t)*t*t*t*255);
    uchar g = (uchar)(15*(1-t)*(1-t)*t*t*255);
    uchar b = (uchar)(8.5*(1-t)*(1-t)*(1-t)*t*255);

    int idx = (gy * width + gx) * 3;
    output[idx] = r;
    output[idx+1] = g;
    output[idx+2] = b;
}

__kernel void julia_kernel(
    __global uchar *output,
    const int width,
    const int height,
    const double center_x,
    const double center_y,
    const double scale,
    const int max_iter,
    const double c_real,
    const double c_imag
) {
    int gx = get_global_id(0);
    int gy = get_global_id(1);
    if (gx >= width || gy >= height) return;

    double x = center_x + (gx - width / 2.0) * scale / width;
    double y = center_y + (gy - height / 2.0) * scale / width;

    int iter = 0;
    while (x*x + y*y <= 4.0 && iter < max_iter) {
        double xtemp = x*x - y*y + c_real;
        y = 2*x*y + c_imag;
        x = xtemp;
        iter++;
    }

    float t = (float)iter / (float)max_iter;
    uchar r = (uchar)(9*(1-t)*t*t*t*255);
    uchar g = (uchar)(15*(1-t)*(1-t)*t*t*255);
    uchar b = (uchar)(8.5*(1-t)*(1-t)*(1-t)*t*255);

    int idx = (gy * width + gx) * 3;
    output[idx] = r;
    output[idx+1] = g;
    output[idx+2] = b;
}

