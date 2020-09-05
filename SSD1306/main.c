/*
 * SSD1306.c
 *
 * Created: 19/07/2020 11:54:22
 * Author : Joshua
 */ 

#include <avr/io.h>

#include "ssd1306.h"

int main(void)
{
    init_display();
	clear_display();
	
	set_scale(NORMAL);
	plot_str("abc", 0, 0, 3);
	set_scale(LARGE);
	plot_bin(0xAB, 2, 0, 8);

    while (1) 
    {
		
    }
}

