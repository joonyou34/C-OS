#include "gpu.h"
#include <inc/uheap.h>
#include <inc/string.h>

void render_window(struct window* windowbuffer)
{
    acquire_spinlock(&GPU.lock);
    memcpy(&GPU.framebuffer, windowbuffer->buffer, sizeof(windowbuffer->buffer));
    release_spinlock(&GPU.lock);
}
