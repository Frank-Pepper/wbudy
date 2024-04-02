/*****************************************************************************
 *   This example is controlling the RGB LED using the joystick
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"

#include "joystick.h"
#include "rgb.h"

int main (void) {

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    uint8_t joy = 0;

    rgb_init();
    joystick_init();

    while(1) {

        joy = joystick_read();

        if ((joy & JOYSTICK_CENTER) != 0) {
            rgb_setLeds(RGB_RED|RGB_GREEN|RGB_BLUE);
            r = RGB_RED;
            b = RGB_BLUE;
            g = RGB_GREEN;
        }

        if ((joy & JOYSTICK_DOWN) != 0) {
            rgb_setLeds(0);
            r = g = b = 0;
        }

        if ((joy & JOYSTICK_LEFT) != 0) {
            r = !r;
            rgb_setLeds(r|g|b);
        }

        if ((joy & JOYSTICK_UP) != 0) {
            if(g)
                g = 0;
            else
                g = RGB_GREEN;
            rgb_setLeds(r|g|b);
        }

        if ((joy & JOYSTICK_RIGHT) != 0) {
            if(b)
                b = 0;
            else
                b = RGB_BLUE;
            rgb_setLeds(r|g|b);
        }

        /* delay */
        Timer0_Wait(200);
    }

}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
