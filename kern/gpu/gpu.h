#ifndef FOS_KERN_GPU_H
#define FOS_KERN_GPU_H
#include <inc/types.h>
#include <inc/uwindow.h>
#include <inc/color.h>
#include <kern/conc/channel.h>

#define BACK_BUFFER_SIZE 2359296

extern bool GPU_ON;

// the mode info block struct, reference: https://wiki.osdev.org/VESA_Video_Modes
struct vbe_mode_info_structure {
	uint16 attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8 window_a;			// deprecated
	uint8 window_b;			// deprecated
	uint16 granularity;		// deprecated; used while calculating bank numbers
	uint16 window_size;
	uint16 segment_a;
	uint16 segment_b;
	uint32 win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16 pitch;			// number of bytes per horizontal line
	uint16 width;			// width in pixels
	uint16 height;			// height in pixels
	uint8 w_char;			// unused...
	uint8 y_char;			// ...
	uint8 planes;
	uint8 bpp;			// bits per pixel in this mode
	uint8 banks;			// deprecated; total number of banks in this mode
	uint8 memory_model;
	uint8 bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8 image_pages;
	uint8 reserved0;

	uint8 red_mask;
	uint8 red_position;
	uint8 green_mask;
	uint8 green_position;
	uint8 blue_mask;
	uint8 blue_position;
	uint8 reserved_mask;
	uint8 reserved_position;
	uint8 direct_color_attributes;

	uint32 framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32 off_screen_mem_off;
	uint16 off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8 reserved1[206];
};
struct gpu
{
	uint32 framebuffer_size; //the size of framebuffer in bytes
    struct spinlock lock; // the framebuffer lock
    uint8* framebuffer; // the virtual address of the framebuffer
    struct vbe_mode_info_structure *mode_info; // the vesa mode info block
	uint8 back_buffer[BACK_BUFFER_SIZE]; //back buffer for double buffering
} GPU;

// Fonts array for text, credits for the font: https://github.com/dhepper/font8x8
extern uint8 font_data[256][8];

void initialize_gpu(struct vbe_mode_info_structure *VESA_mode_info);
void boot_initialize_gpu(struct vbe_mode_info_structure *VESA_mode_info);

void krender_window(struct window* win);

void kdisplay_backbuffer();

void kclear_back_buffer_grayscale(uint8 grayscale_value);

void kdraw_pixel(uint32 row,uint32 col, color_t color_rgba); // row, col, color
void kdraw_pixel_hex(uint32 row,uint32 col ,uint32 color_hex); // row, col, color

void kdraw_rectangle_filled(uint32 top_left_row, uint32 top_left_col, uint32 bot_right_row, uint32 bot_right_col, color_t color_rgba);
void kdraw_rectangle_filled_hex(uint32 top_left_row, uint32 top_left_col, uint32 bot_right_row, uint32 bot_right_col, uint32 color_hex);

void kdraw_square_filled(uint32 top_left_row, uint32 top_left_col, uint32 size, color_t color_rgba);
void kdraw_square_filled_hex(uint32 top_left_row, uint32 top_left_col, uint32 size, uint32 color_hex);

void kdraw_char(uint32 row, uint32 col, char character, uint32 scale, color_t text_color_rgba, color_t bg_color_rgba);
void kdraw_char_hex(uint32 row, uint32 col, char character, uint32 scale, uint32 text_color_hex, uint32 bg_color_hex);

void kget_display_info(struct display_info* ret);
#endif