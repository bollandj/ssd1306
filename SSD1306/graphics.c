
#include "graphics.h"
#include "graphicsfont.h"

charfunc_t plot_char_functions[2] = {plot_char, plot_char_large};

static uint8_t scale;

// SSD1306 initialisation sequence
const uint8_t init_seq[INIT_SEQ_LEN] PROGMEM =
{
	0xAE, // Display off
	0xD5, // Set display clock
	0x80, // Recommended value
	0xA8, // Set multiplex
	0x1F,
	0xD3, // Set display offset
	0x00,
	0x40, // Zero start line
	0x8D, // Charge pump
	0x14,
	0x20, // Memory mode
	0x01, // Vertical addressing
	0xA1, // 0xA0/0xA1 flip horizontally
	0xC8, // 0xC0/0xC8 flip vertically
	0xDA, // Set comp ins
	0x02,
	0x81, // Set contrast
	0x8F, // 0x00 to 0xFF
	0xD9, // Set pre charge
	0xF1,
	0xDB, // Set vcom detect
	0x40,
	0xA6, // Normal (0xA7 = Inverse)
	0xAF  // Display on
};

// converts abcdefgh to aabbccddeeffgghh
static uint16_t stretch(uint16_t x)
{
	x = (x & 0xF0) << 4 | (x & 0x0F);
	x = (x << 2 | x) & 0x3333;
	x = (x << 1 | x) & 0x5555;
	return x | x << 1;
}

// converts abcdefgh to hgfedcba
static uint8_t reverse(uint8_t x)
{
	x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
	x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
	x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
	return x;
}



void init_display()
{
	i2c_init();
	
	i2c_start(OLED_ADDRESS);
	i2c_write(COMMAND);
	
	for (uint8_t i = 0; i < INIT_SEQ_LEN; i++)
		i2c_write(pgm_read_byte(&init_seq[i]));
	
	i2c_stop();
	
	set_scale(NORMAL);
	set_start_line(0);
}

void clear_display()
{
	i2c_start(OLED_ADDRESS);
	i2c_write(COMMAND);
	// Set col address range
	i2c_write(0x21); i2c_write(0); i2c_write(127);
	// Set page address range
	i2c_write(0x22); i2c_write(0); i2c_write(7);
	i2c_stop();
	// Write the data in 16 32-byte transmissions
	for (uint8_t i = 0; i < 32; i++)
	{
		i2c_start(OLED_ADDRESS);
		i2c_write(DATA);
		for (uint8_t j = 0 ; j < 32; j++) i2c_write(0x00);
		i2c_stop();
	}
}

void set_start_line(uint8_t line)
{
	i2c_start(OLED_ADDRESS);
	i2c_write(COMMAND);
	i2c_write(0x40 + (line<<3));
	i2c_stop();
}

void set_scale(uint8_t _scale)
{
	scale = _scale;
}

// Plot a single 8x6 character onscreen
void plot_char(char c, uint8_t line, uint8_t col)
{	
	i2c_start(OLED_ADDRESS);
	i2c_write(COMMAND);
	// set column address range
	i2c_write(0x21); i2c_write(col); i2c_write(col + CHAR_WIDTH-1);
	// set page address range
	i2c_write(0x22); i2c_write(line); i2c_write(line);
	i2c_stop();
	i2c_start(OLED_ADDRESS);
	i2c_write(DATA);
	
	for (uint8_t seg = 0 ; seg < 5; seg++)
		i2c_write(pgm_read_byte(&font[c*(CHAR_WIDTH-1) + seg]));
		
	i2c_stop();
}

void plot_char_large(char c, uint8_t line, uint8_t col)
{	
	i2c_start(OLED_ADDRESS);
	i2c_write(COMMAND);
	// set column address range
	i2c_write(0x21); i2c_write(col); i2c_write(col + (CHAR_WIDTH * 2) - 1);
	// set page address range
	i2c_write(0x22); i2c_write(line); i2c_write(line + 1);
	i2c_stop();
	i2c_start(OLED_ADDRESS);
	i2c_write(DATA);
	
	for (uint8_t seg = 0; seg < CHAR_WIDTH-1; seg++)
	{
		uint16_t stretchbits = stretch(pgm_read_byte(&font[c*(CHAR_WIDTH-1) + seg]));
		uint8_t stretchbits_L = (uint8_t)(stretchbits & 0x00FF);
		uint8_t stretchbits_H = (uint8_t)(stretchbits >> 8);
				
		i2c_write(stretchbits_L);
		i2c_write(stretchbits_H);
		i2c_write(stretchbits_L);
		i2c_write(stretchbits_H);
	}
	
	i2c_stop();
}

void plot_string(char *str, uint8_t line, uint8_t col, uint8_t len)
{
	uint8_t i = 0;
	char c;
	
	uint8_t column = col * CHAR_WIDTH;
	
	charfunc_t plot_char_function = plot_char_functions[scale];
	
	if (!len) len = 255; // 0 defaults to max length
	
	for (i = 0; (i < len) && (c = *str++); i++, column+=(CHAR_WIDTH<<scale))
		plot_char_function(c, line, column);		
}


const uint32_t factors[10] = {1, 10, 100, 1000, 10000, 100000, 1000000,\
							  10000000, 100000000, 1000000000};

void plot_num(uint32_t num, uint8_t line, uint8_t col, uint8_t digits)
{
	uint8_t z = 0;
	
	uint8_t column = col * CHAR_WIDTH;
	
	charfunc_t plot_char_function = plot_char_functions[scale];
	
	for (int8_t i = digits-1; i >= 0; i--, column+=(CHAR_WIDTH<<scale))
	{
		uint8_t q = num / factors[i];
		
		if (q)
		{
			if (q > 9) q = 9;
			plot_char_function('0' + q, line, column);
			num -= q * factors[i];
			z = 1;
		}
		else
		{
			if (z || !i) plot_char_function('0', line, column);
			else         plot_char_function(' ', line, column);
		}
	}
}

void plot_num_signed(int32_t num, uint8_t line, uint8_t col, uint8_t digits)
{
	uint8_t z = 0;
	char sign;
	
	uint8_t column = col * CHAR_WIDTH;
	
	charfunc_t plot_char_function = plot_char_functions[scale];
	
	if (num < 0)
	{
		sign = '-';
		num = -num;
	}
	else
	{
		sign = ' ';
	}
	
	for (int8_t i = digits-1; i >= 0; i--, column+=(CHAR_WIDTH<<scale))
	{
		uint8_t q = num / factors[i];
		
		if (q)
		{
			if (q > 9) q = 9;
			
			if (!z)
			{
				plot_char_function(sign, line, column);
				column += (CHAR_WIDTH<<scale);
				z = 1;
			}

			plot_char_function('0' + q, line, column);
			num -= q * factors[i];			
		}
		else
		{
			if (z)
			{
				plot_char_function('0', line, column);
			}
			else if (!i)
			{
				plot_char_function(' ', line, column);
				column += (CHAR_WIDTH<<scale);
				plot_char_function('0', line, column);
			}
			else
			{
				plot_char(' ', line, column);
			}
		}
	}
}

// void plot_bin(uint16_t num, uint8_t line, uint8_t col)
// {
// 	plot_char('0', line, col);
// 	col += scale;
// 	plot_char('b', line, col);
// 	col += scale;
// 	
// 	uint8_t i0;
// 	if (num & 0xFF00) i0 = 15; // 16-bit
// 	else			  i0 = 7; // 8-bit
// 	
// 	for (int8_t i = i0; i >= 0; i--)
// 	{
// 		plot_char(((num >> i) & 1) + 0x30, line, col);
// 		col += scale;
// 	}
// }
// 
// void plot_hex(uint16_t num, uint8_t line, uint8_t col)
// {
// 	plot_char('0', line, col);
// 	col += scale;
// 	plot_char('x', line, col);
// 	col += scale;
// 	
// 	uint8_t i0;
// 	if (num & 0xFF00) i0 = 3; // 16-bit
// 	else			  i0 = 1; // 8-bit
// 	
// 	col += scale * (i0 + 1); // move to end and plot backwards
// 	
// 	uint8_t nibble;
// 	for (int8_t i = i0; i >= 0; i--)
// 	{
// 		col -= scale;
// 		nibble = num & 0x000F;
// 		
// 		if (nibble <= 9) plot_char(nibble + 0x30, line, col);
// 		else			 plot_char(nibble + 0x37, line, col);
// 		
// 		num >>= 4;
// 	}
// }

// void plot_block(uint8_t c, uint8_t line, uint8_t col)
// {
// 	i2c_start(OLED_ADDRESS);
//
// 	i2c_write(COMMAND);
// 	// Set col address range
// 	i2c_write(0x21); i2c_write(col<<3); i2c_write((col<<3) + 7);
// 	// Set page address range
// 	i2c_write(0x22); i2c_write(line); i2c_write(line);
// 	i2c_stop();
// 	i2c_start(OLED_ADDRESS);
// 	i2c_write(DATA);
//
// 	uint8_t bits = reverse((1<<c) - 1);
// 	for (uint8_t col = 0 ; col < 8; col++)
// 		i2c_write(bits);
//
// 	i2c_stop();
// }
//
// void plot_block_thin(uint8_t c, uint8_t line, uint8_t col)
// {
// 	i2c_start(OLED_ADDRESS);
//
// 	i2c_write(COMMAND);
// 	// Set col address range
// 	i2c_write(0x21); i2c_write(col<<2); i2c_write((col<<2) + 3);
// 	// Set page address range
// 	i2c_write(0x22); i2c_write(line); i2c_write(line);
// 	i2c_stop();
// 	i2c_start(OLED_ADDRESS);
// 	i2c_write(DATA);
//
// 	uint8_t bits = reverse((1<<c) - 1);
// 	for (uint8_t col = 0 ; col < 4; col++)
// 		i2c_write(bits);
//
// 	i2c_stop();
// }
//
// void plot_bar(uint16_t value, uint8_t col)
// {
// 	if (value > 255) value = 255;
// 	value >>= 3; // 0-255 to 0-32
// 	uint8_t rows = value >> 3; // 0-32 to 0-3
// 	uint8_t remainder = value & 0x07;
//
// 	uint8_t i;
// 	for (i = 0; i < rows; i++)
// 	{
// 		plot_block(8, (3-i), col);
// 	}
// 	for (i = rows; i < 4; i++)
// 	{
// 		plot_block(0, (3-i), col);
// 	}
// 	plot_block(remainder, (3-rows), col);
// }
//
// void plot_bar_thin(uint16_t value, uint8_t col)
// {
// 	if (value > 255) value = 255;
// 	value >>= 3; // 0-255 to 0-32
// 	uint8_t rows = value >> 3; // 0-32 to 0-3
// 	uint8_t remainder = value & 0x07;
//
// 	uint8_t i;
// 	for (i = 0; i < rows; i++)
// 	{
// 		plot_block_thin(8, (3-i), col);
// 	}
// 	for (i = rows; i < 4; i++)
// 	{
// 		plot_block_thin(0, (3-i), col);
// 	}
// 	plot_block_thin(remainder, (3-rows), col);
// }