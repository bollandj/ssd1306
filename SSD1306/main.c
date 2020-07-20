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
	set_start_line(0);
	
	plot_num_16(10, 1, 0, 4);

    while (1) 
    {
		
    }
}

