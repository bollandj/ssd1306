
#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "i2cmaster.h"
#include "graphicsfont.h"

#define OLED_ADDRESS 0x78 // left aligned

// Initialisation sequence
#define INIT_SEQ_LEN 24
extern const uint8_t init_seq[INIT_SEQ_LEN] PROGMEM;

#define DATA 0x40
#define SINGLE 0x80
#define COMMAND 0x00

/* Helper functions */
// static uint16_t stretch(uint16_t x);
// static uint8_t reverse(uint8_t x);

#define NORMAL 0
#define LARGE 1

typedef void (*charfunc_t)(char, uint8_t, uint8_t);

extern charfunc_t plot_char_functions[2];

// static uint8_t scale;

extern void init_display();
extern void clear_display();

extern void set_start_line(uint8_t line);
extern void set_scale(uint8_t _scale);

extern void plot_char(char c, uint8_t line, uint8_t col);
extern void plot_char_large(char c, uint8_t line, uint8_t col);

extern void plot_string(char *str, uint8_t line, uint8_t col, uint8_t len);

extern const uint32_t factors[10];
extern void plot_num(uint32_t num, uint8_t line, uint8_t col, uint8_t digits);
extern void plot_num_signed(int32_t num, uint8_t line, uint8_t col, uint8_t digits);


// extern void plot_bin(uint16_t num, uint8_t line, uint8_t col, uint8_t digits);
// 
// extern void plot_hex(uint16_t num, uint8_t line, uint8_t col, uint8_t digits);
//
// extern void plot_block(uint8_t c, uint8_t line, uint8_t col);
//
// extern void plot_block_thin(uint8_t c, uint8_t line, uint8_t col);
//
// extern void plot_bar(uint16_t value, uint8_t col);
//
// extern void plot_bar_thin(uint16_t value, uint8_t col);


#endif /* GRAPHICS_H_ */