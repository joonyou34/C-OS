#ifndef FOS_KERN_GPU_H
#define FOS_KERN_GPU_H
#include <inc/types.h>
#include <inc/vesa_info.h>

struct gpu
{
    struct vbe_mode_info_structure *vbe_mode;
    uint8 framebuffer;
    int lock;
};

void draw_pixel(uint8 *fb, uint32 width, uint32 height, uint32 x, uint32 y, uint32 r, uint32 g, uint32 b);

void display_process_changes(uint8 *fb);

#endif