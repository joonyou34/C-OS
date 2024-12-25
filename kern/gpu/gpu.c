#include "gpu.h"
#include <inc/uheap.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <kern/mem/memory_manager.h>

// draw the colors on the user window buffer into the framebuffer
void krender_window(struct window* win)
{
    acquire_spinlock(&GPU.lock);
    // we get the min to handle when the user window is bigger than the kernel window
    // in that case it'd overflow outside the display
    uint32 min_w = MIN(win->width, GPU.mode_info->width);
    uint32 min_h = MIN(win->height, GPU.mode_info->height);
    uint32 win_width = win->width;
    for (int row = 0; row < min_h; row++)
    {
        for (int col = 0; col < min_w; col++)
        {
            uint32 idx = (row * win_width + col);
            kdraw_pixel_hex(row, col, win->buffer[idx]);
        }
    }
    release_spinlock(&GPU.lock);
}

void initialize_gpu(struct vbe_mode_info_structure *VESA_mode_info) {
    // adding kernel base to the vesa mode info block to be able to use it in protected mdoe
    GPU.mode_info = (struct vbe_mode_info_structure *)(((char *)VESA_mode_info + KERNEL_BASE));
    GPU.framebuffer = (uint8 *)FRAME_BUFFER;

    // the following section is designed to map the physical frames of the framebuffer to a proper virtual address
    // the virtual address chosen is F5C00000 (defined as FRAME_BUFFER), which is located within the kernel region
    // mapped it within the kernel region to not interfer with the user-side
    uint32 buffer_size = (uint32)GPU.mode_info->width * GPU.mode_info->height * (GPU.mode_info->bpp/8);
    uint32 buffer_limit = GPU.mode_info->framebuffer + buffer_size;

    uint32 cur_page = FRAME_BUFFER;

    uint32 *ptr_page_table = NULL;
    get_page_table(ptr_page_directory, cur_page, &ptr_page_table);

    for (uint32 pa = GPU.mode_info->framebuffer; pa < buffer_limit; pa += PAGE_SIZE)
    {
        uint32 pte_available_bits = ptr_page_table[PTX(cur_page)] & PERM_AVAILABLE;
        ptr_page_table[PTX(cur_page)] = CONSTRUCT_ENTRY(pa, pte_available_bits | PERM_WRITEABLE | PERM_PRESENT);

        cur_page += PAGE_SIZE;
    }
    
    // and finally, we initialize the lock for protection since the framebuffer is now a shared resource
    init_spinlock(&GPU.lock, "framesuffer");
}

// draws a pixel on the screen with the color "color"
void kdraw_pixel(uint32 row,uint32 col,color_t color){

    if(col>=GPU.mode_info->width || row>=GPU.mode_info->height ){
        return; //out of window
    }

    uint32 idx = (row * GPU.mode_info->width + col) * 3;

    GPU.framebuffer[idx] = color.blue;   // blue
    GPU.framebuffer[idx + 1] = color.green; // green
    GPU.framebuffer[idx + 2] = color.red; // red

}

// draws a pixel on the screen with the hex color "color"
void kdraw_pixel_hex(uint32 row,uint32 col,uint32 color) {
    kdraw_pixel(row, col, HEX_TO_COLOR(color));
}

// gets the user important VESA info (width, height, bits per pixel)
void kget_display_info(struct display_info* ret) {
    ret->width = GPU.mode_info->width;
    ret->height = GPU.mode_info->height;
    ret->bpp = GPU.mode_info->bpp;
}