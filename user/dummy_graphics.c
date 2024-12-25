#include <inc/lib.h>

void _main(void)
{
	struct window win;
	init_window(&win, 1024, 768);
	for(int row = 0; row < win.height; row++) {
		for(int col = 0; col < win.width; col++) {
			uint8 green = row*255/(win.height-1);
			uint8 red = ((win.height-row-1)*255)/(win.height-1);
			draw_pixel(&win, col, row, MAKE_COLOR(red, green, 0, 255));
		}
	}
	render_window(&win);
	return;
}