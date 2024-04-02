/*****************************************************************************
 *   This example is controlling the LEDs using the joystick
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/



#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_timer.h"

#include "joystick.h"
#include "pca9532.h"



int main (void) {

    uint8_t dir = 0;
    uint32_t delay = 100;
    uint32_t cnt = 0;
    uint16_t ledOn = 0;
    uint16_t ledOff = 0;

    uint8_t joy = 0;

	PINSEL_CFG_Type PinCfg;

	/* Initialize I2C2 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);

	// Initialize I2C peripheral
	I2C_Init(LPC_I2C2, 100000);

	/* Enable I2C1 operation */
	I2C_Cmd(LPC_I2C2, ENABLE);

	pca9532_init();

    joystick_init();

    while (1) {

        joy = joystick_read();

        if ((joy & JOYSTICK_CENTER) != 0) {
            continue;
        }

        if ((joy & JOYSTICK_DOWN) != 0) {
            if (delay < 200)
                delay += 10;
        }

        if ((joy & JOYSTICK_UP) != 0) {
            if (delay > 30)
                delay -= 10;
        }

        if ((joy & JOYSTICK_LEFT) != 0) {
            dir = 0;
        }

        if ((joy & JOYSTICK_RIGHT) != 0) {
            dir = 1;
        }


        if (cnt < 16)
            ledOn |= (1 << cnt);
        if (cnt > 15)
            ledOn &= ~( 1 << (cnt - 16) );

        if (cnt > 15)
            ledOff |= ( 1 << (cnt - 16) );
        if (cnt < 16)
            ledOff &= ~(1 << cnt);

        pca9532_setLeds(ledOn, ledOff);

        if (dir) {
            if (cnt == 0)
                cnt = 31;
            else
                cnt--;

        } else {
            cnt++;
            if (cnt >= 32)
                cnt = 0;
        }

        Timer0_Wait(delay);
    }


}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
