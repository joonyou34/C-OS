#include "gpu.h"

void draw_pixel(uint8 *fb, uint32 width, uint32 height, uint32 x, uint32 y, uint32 r, uint32 g, uint32 b)
{
    if (x >= width || y >= height)
        return;
    uint32 idx = (y * width + x) * 3;
    fb[idx] = r;
    fb[idx + 1] = g;
    fb[idx + 2] = b;
}