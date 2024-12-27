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
#define __COLOR_TO_HEX__(color_rgba) \
    (((uint32)(color_rgba).alpha) | \
    ((uint32)(color_rgba).blue << 8) | \
    ((uint32)(color_rgba).green << 16) | \
    ((uint32)(color_rgba).red << 24))

// transforms from struct color to uint32 (32-bit hex color)
#define COLOR_TO_HEX(color_rgba) __COLOR_TO_HEX__(color_rgba)


// transforms from uint32 (32-bit hex color) to struct color
#define __HEX_TO_COLOR__(hex) \
    (color_t) { \
        .alpha = (hex & 0x000000FF), \
        .blue =  (hex & 0x0000FF00) >> 8, \
        .green = (hex & 0x00FF0000) >> 16, \
        .red =   (hex & 0xFF000000) >> 24 \
    }

// transforms from uint32 (32-bit hex color) to struct color
#define HEX_TO_COLOR(hex) __HEX_TO_COLOR__(hex)


// makes a struct color from given red, green, blue, and alpha values
#define __MAKE_COLOR__(RED, GREEN, BLUE, ALPHA) \
    (color_t) { \
        .alpha = (ALPHA), \
        .blue =  (BLUE), \
        .green = (GREEN), \
        .red =   (RED)\
    }

// makes a struct color from given red, green, blue, and alpha values
#define MAKE_COLOR(RED, GREEN, BLUE, ALPHA) __MAKE_COLOR__(RED, GREEN, BLUE, ALPHA)


// returns a uint32 representing the 32-bit hex color for the given RGB values
#define __MAKE_COLOR_HEX__(RED, GREEN, BLUE, ALPHA) \
    ((ALPHA) | \
    (BLUE << 8) | \
    (GREEN << 16) | \
    (RED << 24))

// returns a uint32 representing the 32-bit hex color for the given RGB values
#define MAKE_COLOR_HEX(RED, GREEN, BLUE, ALPHA) __MAKE_COLOR_HEX__(RED, GREEN, BLUE, ALPHA)

#endif