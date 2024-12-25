#include <inc/types.h>
#include <inc/uheap.h>
#include <inc/uwindow.h>
#include <inc/lib.h>

//===========================
// Initializes a window structure with
// the specified width and height.
// Allocates memory for the window's
// pixel buffer based on the provided 
// returns 1 on fail
//===========================
int init_window(struct window *win, uint16 width, uint16 height)
{
    win->height = height;
    win->width = width;
    win->buffer = (uint32*)malloc((uint32)width * height*sizeof(int));
    if (win->buffer == NULL)
    {
        return 1;
    }
    return 0; // success
}

//===========================
// Draws a pixel at a given x-coordinate
// and y-coordinate with a color of the hex color
// value given
//===========================
void draw_pixel_hex(struct window* win, uint16 col, uint16 row, uint32 hex)
{
    if (col >= win->width || row >= win->height || row < 0 || col < 0)
        return; // out of window
    
    uint32 idx = ((uint32)row * win->width + col);
    win->buffer[idx] = hex;
}

//===========================
// Draws a pixel at a given x-coordinate
// and y-coordinate with a color of the rgba color
// value given
//===========================
void draw_pixel(struct window* win, uint16 col, uint16 row, color_t color_rgba) {
    draw_pixel_hex(win, col, row, COLOR_TO_HEX(color_rgba));
}


//===========================
// renders the given window to the screen
//===========================
void render_window(struct window* win) {
    sys_render_window(win);
}

//===========================
// gets the display info (width, height, bits per pixel)
//===========================
struct display_info get_display_info() {
    struct display_info ret;
    sys_get_display_info(&ret);
    return ret;
}