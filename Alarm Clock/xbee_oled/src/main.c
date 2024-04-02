/*****************************************************************************
 *   This project contains a simple example only trying to contact
 *   the Xbee module. The result is presented on the OLED display.
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_timer.h"

#include "oled.h"

#define UART_DEV LPC_UART3

static void init_uart(void)
{
	PINSEL_CFG_Type PinCfg;
	UART_CFG_Type uartCfg;

	/* Initialize UART3 pin connect */
	PinCfg.Funcnum = 2;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	uartCfg.Baud_rate = 9600;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;

	UART_Init(UART_DEV, &uartCfg);

	UART_TxCmd(UART_DEV, ENABLE);

}


static void init_ssp(void)
{
	SSP_CFG_Type SSP_ConfigStruct;
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize SPI pin connect
	 * P0.7 - SCK;
	 * P0.8 - MISO
	 * P0.9 - MOSI
	 * P2.2 - SSEL - used as GPIO
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 7;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 8;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 9;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Funcnum = 0;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	SSP_ConfigStructInit(&SSP_ConfigStruct);

	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP1, &SSP_ConfigStruct);

	// Enable SSP peripheral
	SSP_Cmd(LPC_SSP1, ENABLE);

}


int main (void) {

    uint8_t buf[100];
    uint32_t len = 0;
    uint8_t row = 1;
    uint32_t pos = 0;
    uint32_t startPos = 0;
    uint8_t ch = 0;
    uint32_t time = 0;


    init_ssp();
    init_uart();

    oled_init();


    oled_clearScreen(OLED_COLOR_WHITE);

    oled_putString(1,1,  (uint8_t*)"Contacting XBee", OLED_COLOR_BLACK, OLED_COLOR_WHITE);

    UART_SendString(UART_DEV,(uint8_t*)"+++");
    Timer0_Wait(1100);
    UART_SendString(UART_DEV,(uint8_t*)"ATVL\r\n");

    while (1) {
        len = UART_Receive(UART_DEV, &buf[pos], 1, NONE_BLOCKING);


        if (len > 0) {
            if (buf[pos] == '\r' || buf[pos] == '\n') {
                buf[pos] = '\0';
                oled_putString(1,1+row*8,  &buf[startPos], OLED_COLOR_BLACK, OLED_COLOR_WHITE);
                row++;
                startPos = pos+1;
            }
            else if ((pos - startPos) == 15) {
                ch = buf[pos];
                buf[pos] = '\0';
                oled_putString(1,1+row*8,  &buf[startPos], OLED_COLOR_BLACK, OLED_COLOR_WHITE);
                buf[pos] = ch;
                startPos = pos;
                row++;
            }
        }

        pos += len;

        if (row > 5 || len > 60)
            break;

        Timer0_Wait(1);
        time++;

        if (time > 3000)
            break;
    }


    if ((row < 5 && len < 60) || time >= 3000)
        oled_putString(1,1+row*8,  (uint8_t*)"No contact!", OLED_COLOR_BLACK, OLED_COLOR_WHITE);

    while(1);

}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
