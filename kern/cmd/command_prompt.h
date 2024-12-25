#ifndef FOS_KERN_MONITOR_H
#define FOS_KERN_MONITOR_H
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/disk.h>
#include <inc/error.h>

// Fonts array for GUI cmd
extern uint8 font_data[128][8];

/*2024*/
//Structure for commands list
typedef LIST_ENTRY(Command) Command_LIST_entry_t;
LIST_HEAD(Command_LIST, Command);

//List of found commands
struct Command_LIST foundCommands;

enum{
CMD_INVALID = -3,
CMD_INV_NUM_ARGS,
CMD_MATCHED,
};
//================================================

// Function to activate the kernel command prompt
int execute_command(char *command_string);
void command_prompt_readline(const char *, char *);
void get_into_prompt() __attribute__((noreturn));

// GUI command prompt functions
void GUI_command_prompt_readline(const char *,char *);
// void draw_char(int c);

/*2024*/
int process_command(int number_of_arguments, char** arguments);


#endif	// !FOS_KERN_MONITOR_H
