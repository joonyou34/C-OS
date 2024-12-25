// Simple command-line kernel prompt useful for
// controlling the kernel and exploring the system interactively.



#include <kern/cmd/command_prompt.h>
#include <kern/proc/user_environment.h>
#include <kern/trap/kdebug.h>
#include <kern/cons/console.h>
#include <kern/tests/tst_handler.h>
#include <kern/cpu/cpu.h>
#include <kern/cpu/sched.h>
#include "commands.h"
#include<kern/gpu/gpu.h>

// ********** This DosKey supported readline function is implemented by **********
// ********** Abdullah Najuib ( FCIS T.A.), 3rd year student, FCIS, 2012

//#define CMD_NUMBER sizeof(comds)/sizeof(comds[0])

#define WHITESPACE "\t\r\n "
#define HISTORY_MAX 19
int last_command_idx = -1;
char command_history[HISTORY_MAX+1][BUFLEN];
char empty[BUFLEN];

const int font_scale = 2;
const int shft_row = 2*8+1;
const int shft_col = 2*8+7;
uint8 font_data[128][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x3E, 0x41, 0x55, 0x41, 0x55, 0x49, 0x3E},
    {0x00, 0x3E, 0x7F, 0x6B, 0x7F, 0x6B, 0x77, 0x3E},
    {0x00, 0x22, 0x77, 0x7F, 0x7F, 0x3E, 0x1C, 0x08},
    {0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x1C, 0x08},
    {0x00, 0x08, 0x1C, 0x2A, 0x7F, 0x2A, 0x08, 0x1C},
    {0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x08, 0x1C},
    {0x00, 0x00, 0x1C, 0x3E, 0x3E, 0x3E, 0x1C, 0x00},
    {0xFF, 0xFF, 0xE3, 0xC1, 0xC1, 0xC1, 0xE3, 0xFF},
    {0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00},
    {0xFF, 0xFF, 0xE3, 0xDD, 0xDD, 0xDD, 0xE3, 0xFF},
    {0x00, 0x0F, 0x03, 0x05, 0x39, 0x48, 0x48, 0x30},
    {0x00, 0x08, 0x3E, 0x08, 0x1C, 0x22, 0x22, 0x1C},
    {0x00, 0x18, 0x14, 0x10, 0x10, 0x30, 0x70, 0x60},
    {0x00, 0x0F, 0x19, 0x11, 0x13, 0x37, 0x76, 0x60},
    {0x00, 0x08, 0x2A, 0x1C, 0x77, 0x1C, 0x2A, 0x08},
    {0x00, 0x60, 0x78, 0x7E, 0x7F, 0x7E, 0x78, 0x60},
    {0x00, 0x03, 0x0F, 0x3F, 0x7F, 0x3F, 0x0F, 0x03},
    {0x00, 0x08, 0x1C, 0x2A, 0x08, 0x2A, 0x1C, 0x08},
    {0x00, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x66},
    {0x00, 0x3F, 0x65, 0x65, 0x3D, 0x05, 0x05, 0x05},
    {0x00, 0x0C, 0x32, 0x48, 0x24, 0x12, 0x4C, 0x30},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x7F},
    {0x00, 0x08, 0x1C, 0x2A, 0x08, 0x2A, 0x1C, 0x3E},
    {0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x1C, 0x1C, 0x1C},
    {0x00, 0x1C, 0x1C, 0x1C, 0x7F, 0x3E, 0x1C, 0x08},
    {0x00, 0x08, 0x0C, 0x7E, 0x7F, 0x7E, 0x0C, 0x08},
    {0x00, 0x08, 0x18, 0x3F, 0x7F, 0x3F, 0x18, 0x08},
    {0x00, 0x00, 0x00, 0x70, 0x70, 0x70, 0x7F, 0x7F},
    {0x00, 0x00, 0x14, 0x22, 0x7F, 0x22, 0x14, 0x00},
    {0x00, 0x08, 0x1C, 0x1C, 0x3E, 0x3E, 0x7F, 0x7F},
    {0x00, 0x7F, 0x7F, 0x3E, 0x3E, 0x1C, 0x1C, 0x08},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18},
    {0x00, 0x36, 0x36, 0x14, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36},
    {0x00, 0x08, 0x1E, 0x20, 0x1C, 0x02, 0x3C, 0x08},
    {0x00, 0x60, 0x66, 0x0C, 0x18, 0x30, 0x66, 0x06},
    {0x00, 0x3C, 0x66, 0x3C, 0x28, 0x65, 0x66, 0x3F},
    {0x00, 0x18, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00},
    {0x00, 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60},
    {0x00, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06},
    {0x00, 0x00, 0x36, 0x1C, 0x7F, 0x1C, 0x36, 0x00},
    {0x00, 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x60},
    {0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60},
    {0x00, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00},
    {0x00, 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C},
    {0x00, 0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E},
    {0x00, 0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E},
    {0x00, 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C},
    {0x00, 0x0C, 0x1C, 0x2C, 0x4C, 0x7E, 0x0C, 0x0C},
    {0x00, 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C},
    {0x00, 0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C},
    {0x00, 0x7E, 0x66, 0x0C, 0x0C, 0x18, 0x18, 0x18},
    {0x00, 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C},
    {0x00, 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C},
    {0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00},
    {0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30},
    {0x00, 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06},
    {0x00, 0x00, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00},
    {0x00, 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60},
    {0x00, 0x3C, 0x66, 0x06, 0x1C, 0x18, 0x00, 0x18},
    {0x00, 0x38, 0x44, 0x5C, 0x58, 0x42, 0x3C, 0x00},
    {0x00, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66},
    {0x00, 0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C},
    {0x00, 0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C},
    {0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7C},
    {0x00, 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E},
    {0x00, 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60},
    {0x00, 0x3C, 0x66, 0x60, 0x60, 0x6E, 0x66, 0x3C},
    {0x00, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66},
    {0x00, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C},
    {0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x6C, 0x6C, 0x38},
    {0x00, 0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66},
    {0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E},
    {0x00, 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63},
    {0x00, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x63, 0x63},
    {0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C},
    {0x00, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x60},
    {0x00, 0x3C, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x06},
    {0x00, 0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66},
    {0x00, 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C},
    {0x00, 0x7E, 0x5A, 0x18, 0x18, 0x18, 0x18, 0x18},
    {0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3E},
    {0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18},
    {0x00, 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63},
    {0x00, 0x63, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x63},
    {0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18},
    {0x00, 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E},
    {0x00, 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E},
    {0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00},
    {0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78},
    {0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F},
    {0x00, 0x0C, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E},
    {0x00, 0x60, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C},
    {0x00, 0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C},
    {0x00, 0x06, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3E},
    {0x00, 0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C},
    {0x00, 0x1C, 0x36, 0x30, 0x30, 0x7C, 0x30, 0x30},
    {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x3C},
    {0x00, 0x60, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66},
    {0x00, 0x00, 0x18, 0x00, 0x18, 0x18, 0x18, 0x3C},
    {0x00, 0x0C, 0x00, 0x0C, 0x0C, 0x6C, 0x6C, 0x38},
    {0x00, 0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66},
    {0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
    {0x00, 0x00, 0x00, 0x63, 0x77, 0x7F, 0x6B, 0x6B},
    {0x00, 0x00, 0x00, 0x7C, 0x7E, 0x66, 0x66, 0x66},
    {0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C},
    {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
    {0x00, 0x00, 0x3C, 0x6C, 0x6C, 0x3C, 0x0D, 0x0F},
    {0x00, 0x00, 0x00, 0x7C, 0x66, 0x66, 0x60, 0x60},
    {0x00, 0x00, 0x00, 0x3E, 0x40, 0x3C, 0x02, 0x7C},
    {0x00, 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x18},
    {0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E},
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x3C, 0x18},
    {0x00, 0x00, 0x00, 0x63, 0x6B, 0x6B, 0x6B, 0x3E},
    {0x00, 0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66},
    {0x00, 0x00, 0x00, 0x66, 0x66, 0x3E, 0x06, 0x3C},
    {0x00, 0x00, 0x00, 0x3C, 0x0C, 0x18, 0x30, 0x3C},
    {0x00, 0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E},
    {0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18},
    {0x00, 0x70, 0x18, 0x18, 0x0C, 0x18, 0x18, 0x70},
    {0x00, 0x00, 0x00, 0x3A, 0x6C, 0x00, 0x00, 0x00},
    {0x00, 0x08, 0x1C, 0x36, 0x63, 0x41, 0x41, 0x7F}};


void draw_char(int y,int x,int index,int scale ){
	if(index>128){
		cprintf("Invalid char\n");
		return;
	}

	for (int row = 0; row < 8; row++) {
			// if ( y + row >= height) break;
		unsigned char row_data = font_data[index][row];
		
		for (int col = 0; col < 8; col++) {


            // if (x + col >= win->width) break;
			
            if (row_data & (1 << (7 - col))) {
				for(int dx= 0 ;dx<scale;dx++){
					for(int dy = 0;dy<scale;dy++){
		                kdraw_pixel_hex(row*scale+x*shft_col,col*scale+y*shft_row, 0xFFFFFFFF);  // Use '#' for filled pixels

					}
				}
            }
        }
    }
}
void clearandwritecommand(int* i, int commandidx, char* buf, int *last_index) {
	for (int j = 0; j < *i; j++) {
		cputchar('\b');
	}
	int len = strlen(command_history[commandidx]);
	memcpy(buf, empty, BUFLEN);
	for (*i = 0; *i < len; (*i)++) {
		cputchar(command_history[commandidx][*i]);
		draw_char((*i),commandidx,(int)command_history[commandidx][*i],font_scale);
		buf[*i] = command_history[commandidx][*i];
	}
	*last_index = len;
}


void RoundAutoCompleteCommandWithTheSamePrefix(int old_buf_len, char* prefix_element,
		char* buf, int* i, int *last_index) {
	for (int j = 0; j < old_buf_len; j++) {
		cputchar('\b');
	}
	int len = strlen(prefix_element);
	memcpy(buf, empty, BUFLEN);
	for (*i = 0; *i < len; (*i)++) {
		cputchar(prefix_element[*i]);
		buf[*i] = prefix_element[*i];
	}
	*last_index = len;
}

char PrefixList[100][1024];
void clear_prefix_list()
{
	for (int i = 0; i < 100; ++i) {
		memset(PrefixList[i], 0, 1024);}
}


void GUI_command_prompt_readline(const char *prompt, char * buf){
		int i, c, echoing, lastIndex;
		if (prompt != NULL)
			cprintf("%s", prompt);
		// GPU
		int commandidx = last_command_idx + 1;
		int prefix_list_idx = lastIndex = i = 0;
		int prefix_list_size, last_c;
		echoing = iscons(0);
		bool is_run_cmd = 0;
		bool is_tst_cmd = 0;

		while (1) {    
			
			//ffff
			c = getchar();
			if (i > lastIndex)
				lastIndex = i;
			
			if (c < 0) {

				if (c != -E_EOF)
					cprintf("read error: %e\n", c);
				return;
			} else if (c == 226) { // Up arrow
				if (commandidx)
					commandidx--;
				clearandwritecommand(&i, commandidx, buf, &lastIndex);
				// clearandwritecommand()
				
			} else if (c == 227) { // Down arrow
				if (commandidx < last_command_idx)
					commandidx++;
				if (last_command_idx >= 0)
					clearandwritecommand(&i, commandidx, buf, &lastIndex);
			} else if (c == 9) { // Tab button
				if (last_c != 9) {
					clear_prefix_list(PrefixList, 100);
					if (strlen(buf) == 0 || last_c == 255)
						continue;
					char *arguments[MAX_ARGUMENTS];
					int number_of_arguments = prefix_list_size = 0;
					char temp_buf[1024];
					strcpy(temp_buf, buf);
					int bufLength = strlen(buf);
					if (buf[bufLength - 1] == ' ')
						continue;
					strsplit(temp_buf, WHITESPACE, arguments, &number_of_arguments);
					int it_str = 0;
					if (number_of_arguments > 1) {
						if((strcmp(arguments[0], "run") != 0) && (strcmp(arguments[0], "load") != 0)
								&& (strcmp(arguments[0], "tst") != 0)) // to autocomplete only in case that the command take arguments and defined arguments (run & load & tst) only
							continue;
						if ((strcmp(arguments[0], "tst") == 0))
						{
							is_tst_cmd = 1;
						}
						else
						{
							is_run_cmd = 1;
						}
						char temp[1024] = "";
						int TotalLen = bufLength - strlen(arguments[number_of_arguments - 1]);
						for (int var = 0; var < TotalLen; ++var) {
							temp[it_str++] = buf[var];
						}
						strcpy(buf, temp);   //buf contains all arguments except the last one
						strcpy(temp_buf, arguments[number_of_arguments - 1]);   //temp_buf contains the last argument
					}
					int it_prefix_list = 0;
					if(number_of_arguments == 1)
					{
						for (int var = 0; var < NUM_OF_COMMANDS; ++var) {
							int x = strncmp(temp_buf, commands[var].name, strlen(temp_buf));
							if (x == 0) {
								it_str = -1;
								char string[1024] = "";
								for (int var3 = 0; var3 < strlen(commands[var].name); ++var3) {
									string[++it_str] = commands[var].name[var3];
								}
								memset(PrefixList[it_prefix_list], 0, 1024);
								strncpy(PrefixList[it_prefix_list], string, it_str + 1);
								it_prefix_list++;
							}
						}
					}
					else
					{
						if(is_run_cmd)
						{
							for (int var = 0; var < NUM_USER_PROGS; ++var) {
								int x = strncmp(temp_buf, ptr_UserPrograms[var].name, strlen(temp_buf));
								if (x == 0) {
									it_str = -1;
									char string[1024] = "";
									if (number_of_arguments > 1) {
										for (int var2 = 0; var2 < strlen(buf); ++var2) {
											string[++it_str] = buf[var2];
										}
									}
									for (int var3 = 0; var3 < strlen(ptr_UserPrograms[var].name) ; ++var3) {
										string[++it_str] = ptr_UserPrograms[var].name[var3];
									}
									memset(PrefixList[it_prefix_list], 0, 1024);
									strncpy(PrefixList[it_prefix_list], string, it_str + 1);
									it_prefix_list++;
								}
							}
						}
						else if(is_tst_cmd)
						{
							for (int var = 0; var < NUM_OF_TESTS; ++var) {
								int x = strncmp(temp_buf, tests[var].name, strlen(temp_buf));
								if (x == 0) {
									it_str = -1;
									char string[1024] = "";
									if (number_of_arguments > 1) {
										for (int var2 = 0; var2 < strlen(buf); ++var2) {
											string[++it_str] = buf[var2];
										}
									}
									for (int var3 = 0; var3 < strlen(tests[var].name) ; ++var3) {
										string[++it_str] = tests[var].name[var3];
									}
									memset(PrefixList[it_prefix_list], 0, 1024);
									strncpy(PrefixList[it_prefix_list], string, it_str + 1);
									it_prefix_list++;
								}
							}
						}
					}
					prefix_list_size = it_prefix_list;
					if (it_prefix_list) {
						prefix_list_idx = it_str = 0;
						for (int var2 = 0; var2 < strlen(PrefixList[0]); ++var2) {
							buf[it_str++] = PrefixList[0][var2];}
						for (int var = 0; var < bufLength; ++var) {
							cputchar('\b');}
						for (int j = 0; j < strlen(buf); ++j) {
							cputchar(buf[j]);
							// draw_char(j*10,last_command_idx*9,(int)buf[j],font_scale);
						}
						i = lastIndex = strlen(buf);
					}
				}
				else {
					if (prefix_list_size > 0) {	int prev = prefix_list_idx;
					prefix_list_idx = (prefix_list_idx + 1) % prefix_list_size;
					RoundAutoCompleteCommandWithTheSamePrefix(strlen(PrefixList[prev]), PrefixList[prefix_list_idx], buf, &i, &lastIndex);
					}
				}
			}

			else if (c == 228) { // left arrow
				if (i > 0) {
					i--;
					cputchar(c);
				}
			} else if (c == 229) { // right arrow
				if (i < lastIndex) {
					i++;
					cputchar(c);
				}
			}
			else if (c == 0xE9 && i > 0) {		 // KEY_DEL
				for (int var = i; var <= lastIndex; ++var) {
					buf[var] = buf[var + 1];
				}
				lastIndex--;
			}
			else if (c >= ' ' && i < BUFLEN - 1 && c != 229 && c != 228) {
				if (echoing)
					cputchar(c);

				buf[i++] = c;
				lastIndex++;
			} else if (c == '\b' && i > 0) {

				if (echoing)
					cputchar(c);
				for (int var = i; var <= i; ++var) {
					buf[var - 1] = buf[var];
				}
				i--;
			} else if (c == '\n' || c == '\r') {

				if (echoing){
					cputchar(c);

				}
				buf[lastIndex] = 0;
				if (last_command_idx == HISTORY_MAX) {
					for (int idx = 0; idx < HISTORY_MAX; idx++) {
						memcpy(command_history[idx], command_history[idx + 1],
								BUFLEN);
					}
					memcpy(command_history[HISTORY_MAX], buf, BUFLEN);
				} else if (strcmp(command_history[last_command_idx], buf) != 0) {
					memcpy(command_history[++last_command_idx], buf, BUFLEN);
				}

				for (int j = 0; j < strlen(buf); ++j) {
					draw_char(j,last_command_idx,(int)buf[j],font_scale);
				}
				return;
				
			}
			last_c = c;

			// for(int row = 0;row<=last_command_idx;row++){
			// 	int len = strlen(command_history[row]);

			// 	for(int col = 0;col<len;col++){
			// 		int c = command_history[row][col];
			// 		draw_char(col*(10),row*(9),c,font_scale);
			// 	}
			// }
		}
}


void command_prompt_readline(const char *prompt, char* buf) {
	int i, c, echoing, lastIndex;
	if (prompt != NULL)
		cprintf("%s", prompt);

	int commandidx = last_command_idx + 1;
	int prefix_list_idx = lastIndex = i = 0;
	int prefix_list_size, last_c;
	echoing = iscons(0);
	bool is_run_cmd = 0;
	bool is_tst_cmd = 0;

	while (1) {    
		//ffff
		c = getchar();
		if (i > lastIndex)
			lastIndex = i;
		if (c < 0) {

			if (c != -E_EOF)
				cprintf("read error: %e\n", c);
			return;
		} else if (c == 226) { // Up arrow
			if (commandidx)
				commandidx--;
			clearandwritecommand(&i, commandidx, buf, &lastIndex);
		} else if (c == 227) { // Down arrow
			if (commandidx < last_command_idx)
				commandidx++;
			if (last_command_idx >= 0)
				clearandwritecommand(&i, commandidx, buf, &lastIndex);
		} else if (c == 9) { // Tab button
			if (last_c != 9) {
				clear_prefix_list(PrefixList, 100);
				if (strlen(buf) == 0 || last_c == 255)
					continue;
				char *arguments[MAX_ARGUMENTS];
				int number_of_arguments = prefix_list_size = 0;
				char temp_buf[1024];
				strcpy(temp_buf, buf);
				int bufLength = strlen(buf);
				if (buf[bufLength - 1] == ' ')
					continue;
				strsplit(temp_buf, WHITESPACE, arguments, &number_of_arguments);
				int it_str = 0;
				if (number_of_arguments > 1) {
					if((strcmp(arguments[0], "run") != 0) && (strcmp(arguments[0], "load") != 0)
							&& (strcmp(arguments[0], "tst") != 0)) // to autocomplete only in case that the command take arguments and defined arguments (run & load & tst) only
						continue;
					if ((strcmp(arguments[0], "tst") == 0))
					{
						is_tst_cmd = 1;
					}
					else
					{
						is_run_cmd = 1;
					}
					char temp[1024] = "";
					int TotalLen = bufLength - strlen(arguments[number_of_arguments - 1]);
					for (int var = 0; var < TotalLen; ++var) {
						temp[it_str++] = buf[var];
					}
					strcpy(buf, temp);   //buf contains all arguments except the last one
					strcpy(temp_buf, arguments[number_of_arguments - 1]);   //temp_buf contains the last argument
				}
				int it_prefix_list = 0;
				if(number_of_arguments == 1)
				{
					for (int var = 0; var < NUM_OF_COMMANDS; ++var) {
						int x = strncmp(temp_buf, commands[var].name, strlen(temp_buf));
						if (x == 0) {
							it_str = -1;
							char string[1024] = "";
							for (int var3 = 0; var3 < strlen(commands[var].name); ++var3) {
								string[++it_str] = commands[var].name[var3];
							}
							memset(PrefixList[it_prefix_list], 0, 1024);
							strncpy(PrefixList[it_prefix_list], string, it_str + 1);
							it_prefix_list++;
						}
					}
				}
				else
				{
					if(is_run_cmd)
					{
						for (int var = 0; var < NUM_USER_PROGS; ++var) {
							int x = strncmp(temp_buf, ptr_UserPrograms[var].name, strlen(temp_buf));
							if (x == 0) {
								it_str = -1;
								char string[1024] = "";
								if (number_of_arguments > 1) {
									for (int var2 = 0; var2 < strlen(buf); ++var2) {
										string[++it_str] = buf[var2];
									}
								}
								for (int var3 = 0; var3 < strlen(ptr_UserPrograms[var].name) ; ++var3) {
									string[++it_str] = ptr_UserPrograms[var].name[var3];
								}
								memset(PrefixList[it_prefix_list], 0, 1024);
								strncpy(PrefixList[it_prefix_list], string, it_str + 1);
								it_prefix_list++;
							}
						}
					}
					else if(is_tst_cmd)
					{
						for (int var = 0; var < NUM_OF_TESTS; ++var) {
							int x = strncmp(temp_buf, tests[var].name, strlen(temp_buf));
							if (x == 0) {
								it_str = -1;
								char string[1024] = "";
								if (number_of_arguments > 1) {
									for (int var2 = 0; var2 < strlen(buf); ++var2) {
										string[++it_str] = buf[var2];
									}
								}
								for (int var3 = 0; var3 < strlen(tests[var].name) ; ++var3) {
									string[++it_str] = tests[var].name[var3];
								}
								memset(PrefixList[it_prefix_list], 0, 1024);
								strncpy(PrefixList[it_prefix_list], string, it_str + 1);
								it_prefix_list++;
							}
						}
					}
				}
				prefix_list_size = it_prefix_list;
				if (it_prefix_list) {
					prefix_list_idx = it_str = 0;
					for (int var2 = 0; var2 < strlen(PrefixList[0]); ++var2) {
						buf[it_str++] = PrefixList[0][var2];}
					for (int var = 0; var < bufLength; ++var) {
						cputchar('\b');}
					for (int j = 0; j < strlen(buf); ++j) {
						cputchar(buf[j]);}
					i = lastIndex = strlen(buf);
				}
			}
			else {
				if (prefix_list_size > 0) {	int prev = prefix_list_idx;
				prefix_list_idx = (prefix_list_idx + 1) % prefix_list_size;
				RoundAutoCompleteCommandWithTheSamePrefix(strlen(PrefixList[prev]), PrefixList[prefix_list_idx], buf, &i, &lastIndex);
				}
			}
		}

		else if (c == 228) { // left arrow
			if (i > 0) {
				i--;
				cputchar(c);
			}
		} else if (c == 229) { // right arrow
			if (i < lastIndex) {
				i++;
				cputchar(c);
			}
		}
		else if (c == 0xE9 && i > 0) {		 // KEY_DEL
			for (int var = i; var <= lastIndex; ++var) {
				buf[var] = buf[var + 1];
			}
			lastIndex--;
		}
		else if (c >= ' ' && i < BUFLEN - 1 && c != 229 && c != 228) {
			if (echoing)
				cputchar(c);
			buf[i++] = c;
			lastIndex++;
		} else if (c == '\b' && i > 0) {

			if (echoing)
				cputchar(c);
			for (int var = i; var <= i; ++var) {
				buf[var - 1] = buf[var];
			}
			i--;
		} else if (c == '\n' || c == '\r') {

			if (echoing)
				cputchar(c);

			buf[lastIndex] = 0;
			if (last_command_idx == HISTORY_MAX) {
				for (int idx = 0; idx < HISTORY_MAX; idx++) {
					memcpy(command_history[idx], command_history[idx + 1],
							BUFLEN);
				}
				memcpy(command_history[HISTORY_MAX], buf, BUFLEN);
			} else if (strcmp(command_history[last_command_idx], buf) != 0) {
				memcpy(command_history[++last_command_idx], buf, BUFLEN);
			}
			return;
			
		}
		last_c = c;
	}
}
// ******************************************************************
// ******************************************************************

extern bool autograde ;
void run_command_prompt()
{
	if (autograde)
	{
		char cmd1_2[BUFLEN] = "tst bsd_nice 0";
		char cmd2_2[BUFLEN] = "tst bsd_nice 1";
		char cmd3_2[BUFLEN] = "tst bsd_nice 2";
		//execute_command(cmd3_2);
		autograde = 0;
	}
	/*2024*/
	LIST_INIT(&foundCommands);
	//========================

	char command_line[BUFLEN];

	while (1==1)
	{
		//readline("FOS> ", command_line);

		// ********** This DosKey supported readline function is a combined implementation from **********
		// ********** 		Mohamed Raafat & Mohamed Yousry, 3rd year students, FCIS, 2017		**********
		// ********** 				Combined, edited and modified by TA\Ghada Hamed				**********
		memset(command_line, 0, sizeof(command_line));
		// command_prompt_readline("FOS> ", command_line);
		GUI_command_prompt_readline("GPU>",command_line);
		//parse and execute the command
		if (command_line != NULL)
			if (execute_command(command_line) < 0)
				break;
	}
}

/* get into the command prompt - This function does not return.
 * The only way to get into the prompt is via this function to ensure correct re-initializations
 * The following variables are used to clear the entire content of the KERNEL STACK before getting into the prompt
 * They're placed globally (instead of locally) to avoid clearing them while they're in use [el7 :)]
 */
int m;
char *p ;
void get_into_prompt()
{
	while (1)
	{
		//disable interrupt if it's already enabled
		if (read_eflags() & FL_IF)
			cli();

		//Switch to the kernel virtual memory
		switchkvm();

		//Reset current CPU
		struct cpu *c = mycpu();
		c->ncli = 0;
		c->intena = 0;
		c->scheduler = NULL;
		c->scheduler_status = SCH_STOPPED ;
		c->proc = NULL;

		//Read current ESP
		uint32 cur_esp = read_esp();
		//cprintf("*** KERNEL SP: BEFORE RESIT = %x - ", cur_esp);

//		//Make sure it's in the correct stack (i.e. KERN STACK below KERN_BASE)
//		assert(cur_esp < SCHD_KERN_STACK_TOP && cur_esp >= SCHD_KERN_STACK_TOP - KERNEL_STACK_SIZE);

		//Reset ESP to the beginning of the SCHED KERNEL STACK of this CPU before getting into the cmd prmpt
		uint32 cpuStackTop = (uint32)c->stack + KERNEL_STACK_SIZE;
		uint32 cpuStackBottom = (uint32)c->stack + PAGE_SIZE/*GUARD Page*/;
		write_esp(cpuStackTop);

		//cprintf("AFTER RESIT = %x ***\n", read_esp());

		//Clear the stack content to avoid any garbage data on it when getting back into prompt
		if (cur_esp < cpuStackTop && cur_esp >= cpuStackBottom)
		{
			//memset((char*)cur_esp, 0, SCHD_KERN_STACK_TOP - cur_esp);
			p = (char*)cur_esp;
			m = cpuStackTop - cur_esp;
			while (--m >= 0)
				*p++ = 0;
		}
		else	//clear the ENTIRE SCHED KERN STACK
		{
			//memset((char*)schd_kern_stack_bottom, 0, SCHD_KERN_STACK_TOP - schd_kern_stack_bottom);
			p = (char*)cpuStackBottom;
			m = cpuStackTop - cpuStackBottom;
			while (--m >= 0)
				*p++ = 0;
		}

		//Reset EBP to ZERO so that when calling the run_command_prompt() it pushes ZERO into the stack
		write_ebp(0);

		//Get into the prompt (should NOT return)
		run_command_prompt(NULL);
	}

}


/***** Kernel command prompt command interpreter *****/

//define the white-space symbols
#define WHITESPACE "\t\r\n "

//Function to parse any command and execute it
//(simply by calling its corresponding function)
int execute_command(char *command_string)
{
	// Split the command string into whitespace-separated arguments
	int number_of_arguments;
	//allocate array of char * of size MAX_ARGUMENTS = 16 found in string.h
	char *arguments[MAX_ARGUMENTS];


	strsplit(command_string, WHITESPACE, arguments, &number_of_arguments) ;
	if (number_of_arguments == 0)
		return 0;

	int ret = process_command(number_of_arguments, arguments);

	//cprintf("cmd %s, num of args %d, return %d\n", arguments[0], number_of_arguments, ret);

	if (ret == CMD_INVALID)
	{
		cprintf("Unknown command '%s'\n", arguments[0]);
	}
	else if (ret == CMD_INV_NUM_ARGS)
	{
		int numOfFoundCmds = LIST_SIZE(&foundCommands);
		if (numOfFoundCmds != 1)
		{
			panic("command is found but the list is either empty or contains more than one command!");
		}
		struct Command * cmd = LIST_FIRST(&foundCommands);
		cprintf("%s: invalid number of args.\nDescription: %s\n", cmd->name, cmd->description);
	}
	else if (ret == CMD_MATCHED)
	{
		int i = 1;
		int numOfFoundCmds = LIST_SIZE(&foundCommands);
		if (numOfFoundCmds == 0)
		{
			panic("command is matched but the list is empty!");
		}
		struct Command * cmd = NULL;
		LIST_FOREACH(cmd, &foundCommands)
		{
			cprintf("[%d] %s\n", i++, cmd->name);
		}
		cprintf("Please select the required command [1] to [%d] and press enter? or press any other key to cancel: ", numOfFoundCmds);
		char Chose = getchar();
		cputchar(Chose);
		int selection = 0;
		while (Chose >= '0' && Chose <= '9')
		{
			selection = selection*10 + (Chose - '0') ;
			if (selection < 1 || selection > numOfFoundCmds)
				break;

			Chose = getchar();
			cputchar(Chose);
		}
		cputchar('\n');
		if (selection >= 1 && selection <= numOfFoundCmds)
		{
			int c = 1;
			LIST_FOREACH(cmd, &foundCommands)
			{
				if (c++ == selection)
				{
					if (cmd->num_of_args == 0)
					{
						cprintf("FOS> %s\n", cmd->name);
						return cmd->function_to_execute(number_of_arguments, arguments);
					}
					else
					{
						cprintf("%s: %s\n", cmd->name, cmd->description);
						return 0;
					}
				}
			}
		}
	}
	else
	{
		return commands[ret].function_to_execute(number_of_arguments, arguments);
	}
	return 0;
}

bool FindSubsequencePLS(const char *sting1 ,const char *sting2){
    while(*sting1 != '\0' && sting2 !='\0'){
        if(*sting1 == *sting2){
            sting2++;
        }
        sting1++;
    }

    return *sting2=='\0';
}

int process_command(int number_of_arguments, char **arguments)
{
    // TODO: [PROJECT'24.MS1 - #01] [1] PLAY WITH CODE! - process_command
	LIST_INIT(&foundCommands);

    for (int i = 0; i < NUM_OF_COMMANDS; i++)
    {
        if (strcmp(arguments[0], commands[i].name) == 0)
        {
            if(commands[i].num_of_args == -1 && number_of_arguments >= 2)
            {
                return i;
            }
            else if (number_of_arguments - 1 == commands[i].num_of_args)
            {
                return i;
            }
            else
            {
                LIST_INSERT_HEAD(&foundCommands, &commands[i]);
                return CMD_INV_NUM_ARGS;
            }
        }
    }

    for (int i = 0; i < NUM_OF_COMMANDS; i++)
    {
        if (FindSubsequencePLS(commands[i].name, arguments[0]))
        {
            LIST_INSERT_HEAD(&foundCommands, &commands[i]);
        }
    }

    if (LIST_SIZE(&foundCommands) != 0)
    {
        return CMD_MATCHED;
    }

    return CMD_INVALID;
}
