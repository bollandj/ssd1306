
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

extern uint8_t scale;

#define DATA 0x40
#define SINGLE 0x80
#define COMMAND 0x00

/* Helper functions */
// static uint16_t stretch(uint16_t x);
// static uint8_t reverse(uint8_t x);

extern void init_display();

extern void clear_display();

extern void set_start_line(uint8_t line);

extern void plot_char(char c, uint8_t line, uint8_t col);
extern void plot_char_large(char c, uint8_t line, uint8_t col);

extern void plot_string(char *str, uint8_t line, uint8_t col, uint8_t len);
extern void plot_string_large(char *str, uint8_t line, uint8_t col, uint8_t len);

extern const uint16_t factors[5];
extern void plot_num(uint16_t num, uint8_t line, uint8_t col, uint8_t digits);
extern void plot_num_signed(uint16_t num, uint8_t line, uint8_t col, uint8_t digits);

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