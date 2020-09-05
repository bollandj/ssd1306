
/**************************************************************************/
/*!
  @file ssd1306.c
  @mainpage SSD1306 Library
  @section intro Introduction
  
  @section author Author
  
  @section license License
  
 */
/**************************************************************************/

#include "ssd1306.h"
#include "font.h"

charfunc_t plot_char_functions[2] = {plot_char_normal, plot_char_large};
charfunc_t plot_char;
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

/*!
 * @brief Converts bit pattern abcdefgh to aabbccddeeffgghh
 * @param x input byte to be stretched
 */
static uint16_t stretch(uint16_t x)
{
	x = (x & 0xF0) << 4 | (x & 0x0F);
	x = (x << 2 | x) & 0x3333;
	x = (x << 1 | x) & 0x5555;
	return x | x << 1;
}

/*!
 * @brief Converts bit pattern abcdefgh to hgfedcba
 * @param x input byte to be reversed
 */
static uint8_t reverse(uint8_t x)
{
	x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
	x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
	x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
	return x;
}

/*!
 * @brief Initialise display by sending initialisation command sequence (init_seq)
 */
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

/*!
 * @brief Blank all display pixels
 */
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

/*!
 * @brief Set the line of the display RAM that corresponds to the first line of the display
 * @param line start line
 */
void set_start_line(uint8_t line)
{
	i2c_start(OLED_ADDRESS);
	i2c_write(COMMAND);
	i2c_write(0x40 + (line<<3));
	i2c_stop();
}

/*!
 * @brief Set the scale of displayed characters
 * @param _scale character height (NORMAL/0 = 1 line, LARGE/1 = 2 lines)
 */
void set_scale(uint8_t _scale)
{
	scale = _scale;
	plot_char = plot_char_functions[scale];
}

/*!
 * @brief Plot a single character onscreen (NORMAL scale)
 * @param c character to be plotted
 * @param line line on which character is to be plotted
 * @param col start column for character (0 - DISPLAY_WIDTH-1)
 */
void plot_char_normal(char c, uint8_t line, uint8_t col)
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

/*!
 * @brief Plot a single character onscreen (LARGE scale)
 * @param c character to be plotted
 * @param line start line on which character is to be plotted (line+1 will also be occupied)
 * @param col start column for character (0 - DISPLAY_WIDTH-1)
 */
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

/*!
 * @brief Plot an ASCII string onscreen
 * @param str string to be plotted
 * @param line line on which string is to be plotted
 * @param col start column (0 - floor(DISPLAY_WIDTH/CHAR_WIDTH)-1) 
 * @param len maximum characters to plot (0 = no limit)
 */
void plot_str(char *str, uint8_t line, uint8_t col, uint8_t len)
{
	uint8_t i = 0;
	char c;
	
	uint8_t column = col * CHAR_WIDTH;
		
	if (!len) len = 255; // 0 defaults to max length
	
	for (i = 0; (i < len) && (c = *str++); i++)
	{
		plot_char(c, line, column);
		column += (CHAR_WIDTH << scale);
	}
}


const uint32_t factors[10] = {1, 10, 100, 1000, 10000, 100000, 1000000,\
							  10000000, 100000000, 1000000000};

/*!
 * @brief Plot an unsigned integer
 * @param num number to be plotted
 * @param line line on which number is to be plotted
 * @param col start column (0 - floor(DISPLAY_WIDTH/CHAR_WIDTH)-1)
 * @param digits maximum number of digits to expect (this number of characters' worth of
          space will be occupied on the display)
 */
void plot_int(uint32_t num, uint8_t line, uint8_t col, uint8_t digits)
{
	uint8_t z = 0;
	
	uint8_t column = col * CHAR_WIDTH;
	
	for (int8_t i = digits-1; i >= 0; i--)
	{
		uint8_t q = num / factors[i];
		
		if (q)
		{
			if (q > 9) q = 9;
			plot_char('0' + q, line, column);
			num -= q * factors[i];
			z = 1;
		}
		else
		{
			if (z || !i) plot_char('0', line, column);
			else         plot_char(' ', line, column);
		}
		
		column += (CHAR_WIDTH << scale);
	}
}

/*!
 * @brief Plot a signed integer
 * @param num number to be plotted
 * @param line line on which number is to be plotted
 * @param col start column (0 - floor(DISPLAY_WIDTH/CHAR_WIDTH)-1)
 * @param digits maximum number of digits to expect (this number of characters' worth of
          space, plus one more for '-' sign, will be occupied on the display)
 */
void plot_int_signed(int32_t num, uint8_t line, uint8_t col, uint8_t digits)
{
	uint8_t z = 0;
	char sign;
	
	uint8_t column = col * CHAR_WIDTH;
	
	if (num < 0)
	{
		sign = '-';
		num = -num;
	}
	else
	{
		sign = ' ';
	}
	
	for (int8_t i = digits-1; i >= 0; i--)
	{
		uint8_t q = num / factors[i];
		
		if (q)
		{
			if (q > 9) q = 9;
			
			if (!z)
			{
				plot_char(sign, line, column);
				column += (CHAR_WIDTH << scale);
				z = 1;
			}

			plot_char('0' + q, line, column);
			num -= q * factors[i];			
		}
		else
		{
			if (z)
			{
				plot_char('0', line, column);
			}
			else if (!i)
			{
				plot_char(' ', line, column);
				column += (CHAR_WIDTH << scale);
				plot_char('0', line, column);
			}
			else
			{
				plot_char(' ', line, column);
			}
		}
		
		column += (CHAR_WIDTH << scale);
	}
}

/*!
 * @brief Plot an integer as binary
 * @param num number to be plotted
 * @param line line on which number is to be plotted
 * @param col start column (0 - floor(DISPLAY_WIDTH/CHAR_WIDTH)-1)
 * @param digits maximum number of bits to expect (this number of characters' worth of
          space will be occupied on the display)
 */
void plot_bin(uint32_t num, uint8_t line, uint8_t col, uint8_t bits)
{
	uint8_t column = col * CHAR_WIDTH;
	
	plot_str("0b", line, column, 2);
	column += (CHAR_WIDTH << scale) * (bits + 2); // move to end and plot backwards
	
	uint8_t bit;
	for (uint8_t i = bits; i > 0; i--)
	{
		column -= (CHAR_WIDTH << scale);
		bit = num & 0x00000001;
		
		if (bit) plot_char('1', line, column);
		else	 plot_char('0', line, column);
		
		num >>= 1;
	}
}

/*!
 * @brief Plot an integer as hex
 * @param num number to be plotted
 * @param line line on which number is to be plotted
 * @param col start column (0 - floor(DISPLAY_WIDTH/CHAR_WIDTH)-1)
 * @param digits maximum number of nibbles to expect (this number of characters' worth of
          space will be occupied on the display)
 */
void plot_hex(uint32_t num, uint8_t line, uint8_t col, uint8_t nibbles)
{
	uint8_t column = col * CHAR_WIDTH;
	
	plot_str("0x", line, column, 2);
	column += (CHAR_WIDTH << scale) * (nibbles + 2); // move to end and plot backwards
	
	uint8_t nibble;
	for (uint8_t i = nibbles; i > 0; i--)
	{
		column -= (CHAR_WIDTH << scale);
		nibble = num & 0x0000000F;
		
		if (nibble < 10) plot_char(nibble + 0x30, line, column);
		else			 plot_char(nibble + 0x37, line, column);
		
		num >>= 4;
	}
}

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