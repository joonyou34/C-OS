#ifndef FOS_KERN_GPU_H
#define FOS_KERN_GPU_H
#include <inc/types.h>
#include <inc/vesa_info.h>
#include <inc/uwindow.h>
#include <kern/conc/channel.h>
struct gpu
{
    struct vbe_mode_info_structure *vbe_mode;
    uint8 framebuffer;
    struct spinlock lock;
} GPU;

void render_window(struct window* windowbuffer);
#endif