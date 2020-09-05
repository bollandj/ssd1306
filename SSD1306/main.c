/*
 * SSD1306.c
 *
 * Created: 19/07/2020 11:54:22
 * Author : Joshua
 */ 

#include <avr/io.h>

#include "graphics.h"

int main(void)
{
    init_display();
	clear_display();
	
	set_scale(NORMAL);
	plot_num(12345678, 0, 0, 8);
	set_scale(LARGE);
	plot_num(1234, 2, 0, 4);

    while (1) 
    {
		
    }
}

