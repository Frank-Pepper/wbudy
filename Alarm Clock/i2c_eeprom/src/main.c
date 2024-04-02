/*****************************************************************************
 *   This example shows how to read and write to the eeprom
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_timer.h"
#include "string.h"

#include "eeprom.h"

#define E_WRITE_LEN 200

#define UART_DEV LPC_UART3

static void init_i2c(void)
{
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

}



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

	uartCfg.Baud_rate = 115200;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;

	UART_Init(UART_DEV, &uartCfg);

	UART_TxCmd(UART_DEV, ENABLE);

}



int main (void) {
    int i = 0;
    uint16_t offset = 240;
    uint8_t b[E_WRITE_LEN];
    int16_t len = 0;

    init_i2c();
    init_uart();

    for (i = 0; i < E_WRITE_LEN; i++) {
        b[i] = (uint8_t)(i+6);
    }

    eeprom_init();

    len = eeprom_write(b, offset, E_WRITE_LEN);

    UART_SendString(UART_DEV, (uint8_t*)"EEPROM example\r\n");

    if (len != E_WRITE_LEN) {
    	UART_SendString(UART_DEV, (uint8_t*)"EEPROM: Failed to write data\r\n");
        return 1;
    }

    UART_SendString(UART_DEV, (uint8_t*)"EEPROM: Data written\r\n");
    Timer0_Wait(2000);

    memset(b, 0, E_WRITE_LEN);

    UART_SendString(UART_DEV, (uint8_t*)"EEPROM: Reading\r\n");
    len = eeprom_read(b, offset, E_WRITE_LEN);

    if (len != E_WRITE_LEN) {
    	UART_SendString(UART_DEV, (uint8_t*)"EEPROM: Failed to read all data\r\n");
        return 1;
    }

    UART_SendString(UART_DEV, (uint8_t*)"EEPROM: Verifing\r\n");
    for (i = 0; i < E_WRITE_LEN; i++) {
        if (b[i] != (uint8_t)(i+6)) {

        	UART_SendString(UART_DEV, (uint8_t*)"EEPROM: Invalid data\r\n");

            return 1;
        }
    }

    UART_SendString(UART_DEV, (uint8_t*)"EEPROM: OK\r\n");

    return 0;


}



void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
