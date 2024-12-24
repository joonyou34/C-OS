#ifndef FOS_INC_UWINDOW_H
#define FOS_INC_UWINDOW_H 1

struct window
{
    uint32 *buffer;
    uint32 width;
    uint32 height;
};

int init_window(struct window *win, uint32 width, uint32 height);

void draw_pixel(uint8 *fb, uint32 width, uint32 height, uint32 x, uint32 y, uint32 r, uint32 g, uint32 b);

#endif
