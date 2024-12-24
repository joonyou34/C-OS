#include <inc/types.h>
#include <inc/uheap.h>
#include <inc/uwindow.h>

//===========================
// Initializes a window structure with
// the specified width and height.
// Allocates memory for the window's
// pixel buffer based on the provided 
//===========================
int init_window(struct window *win, uint32 width, uint32 height)
{
    win->height = height;
    win->width = width;
    win->buffer = (uint32*)malloc(width * height);
    if (win->buffer == NULL)
    {
        return 0;
    }
    return 1; // success
}

//===========================
// Draws a pixel at a given x-coordinate
// and y-coordinate with a color of the rgb
// value given
//===========================
void draw_pixel(uint8 *fb, uint32 width, uint32 height, uint32 x, uint32 y, uint32 r, uint32 g, uint32 b)
{
    if (x >= width || y >= height)
        return;
    uint32 idx = (y * width + x) * 3;
    fb[idx] = r;
    fb[idx + 1] = g;
    fb[idx + 2] = b;
}