#ifndef FOS_INC_COLOR_H
#define FOS_INC_COLOR_H 1
#include <inc/types.h>
struct color {
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 alpha;
};

typedef struct color color_t;

// transforms from struct color to uint32 (32-bit hex color)
#define COLOR_TO_HEX(color_rgba) \
    (((uint32)(color_rgba).alpha) | \
    ((uint32)(color_rgba).blue << 8) | \
    ((uint32)(color_rgba).green << 16) | \
    ((uint32)(color_rgba).red << 24))

// transforms from uint32 (32-bit hex color) to struct color
#define HEX_TO_COLOR(hex) \
    (color_t) { \
        .alpha = (hex & 0x000000FF), \
        .blue =  (hex & 0x0000FF00) >> 8, \
        .green = (hex & 0x00FF0000) >> 16, \
        .red =   (hex & 0xFF000000) >> 24 \
    }

// makes a struct color from given red, green, blue, and alpha values
#define MAKE_COLOR(RED, GREEN, BLUE, ALPHA) \
    (color_t) { \
        .alpha = (ALPHA), \
        .blue =  (BLUE), \
        .green = (GREEN), \
        .red =   (RED)\
    }
#endif