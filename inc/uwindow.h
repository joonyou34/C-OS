#ifndef FOS_INC_UWINDOW_H
#define FOS_INC_UWINDOW_H 1

#include <inc/color.h>
struct window
{
    uint32 *buffer; // the buffer holding the pixel data, size is width*height
    uint16 width;
    uint16 height;
};

struct display_info {
    uint16 width;
    uint16 height;
    uint8 bpp; //bits per pixel (how many bits per color/pixel)
};

int init_window(struct window* win, uint16 width, uint16 height);
void draw_pixel(struct window* win, uint16 col, uint16 row, color_t color_rgba);
void draw_pixel_hex(struct window* win, uint16 col, uint16 row, uint32 color_hex);
void render_window(struct window* win);


#endif
