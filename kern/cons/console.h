/* See COPYRIGHT for copyright information. */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif

#include <inc/types.h>
#include <kern/conc/sleeplock.h>
#include <kern/conc/ksemaphore.h>
#include <kern/gpu/gpu.h>

#define MONO_BASE	0x3B4
#define MONO_BUF	0xB0000
#define CGA_BASE	0x3D4
#define CGA_BUF		0xB8000

#define CRT_ROWS	25
#define CRT_COLS	80
#define CRT_SIZE	(CRT_ROWS * CRT_COLS)

#define LEFT_ARROW 228
#define RIGHT_ARROW 229

#define FONT_SCALE 2
#define CURSOR_SPACING 1
#define TAB_SPACE 5
#define GUI_CMD_BUFFER_SIZE 1048576
extern const uint32 SHIFT_ROW;
extern const uint32 SHIFT_COL;
extern uint32 console_text_color_hex;
extern uint32 console_bg_color_hex;
extern uint8 gui_cmd_buffer[GUI_CMD_BUFFER_SIZE];
extern uint8 tst_buff[GUI_CMD_BUFFER_SIZE];

void draw_gui_console();

void cons_init(void);
void cons_putc(int c);
int cons_getc(void);
int cons_getc2(void);

/*2024*/

#define CONS_LCK_METHOD LCK_INT			//Specify the method of LOCK to protect the console
void cons_lock(void);					//lock the console so that no other processes can deal with it (either read from KB or print no screen)
void cons_unlock(void);					//unlock the console so that other processes can deal with it
struct sleeplock conslock;				//sleeplock to protect the console
struct ksemaphore conssem;				//semaphore to protect the console

#define KBD_INT_BLK_METHOD LCK_INT 		//Specify the method of handling the block/release on KBD
struct Channel KBDchannel;				//channel of waiting for a char from KB
struct spinlock KBDlock;				//spinlock to protect the KBDchannel
struct ksemaphore KBDsem;				//semaphore to manage KBD interrupts

void kbd_intr(void); // irq 1
void serial_intr(void); // irq 4
void keyboard_interrupt_handler();

#endif /* _CONSOLE_H_ */
