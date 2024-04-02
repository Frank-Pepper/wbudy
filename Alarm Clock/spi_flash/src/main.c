/*****************************************************************************
 *   This example shows how to read and write to the SPI Flash
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

#include "flash.h"

#define E_WRITE_LEN (264*4)

#define UART_DEV LPC_UART3

static uint8_t buf[10];

static void intToString(int value, uint8_t* pBuf, uint32_t len, uint32_t base)
{
    static const char* pAscii = "0123456789abcdefghijklmnopqrstuvwxyz";
    int pos = 0;
    int tmpValue = value;

    // the buffer must not be null and at least have a length of 2 to handle one
    // digit and null-terminator
    if (pBuf == NULL || len < 2)
    {
        return;
    }

    // a valid base cannot be less than 2 or larger than 36
    // a base value of 2 means binary representation. A value of 1 would mean only zeros
    // a base larger than 36 can only be used if a larger alphabet were used.
    if (base < 2 || base > 36)
    {
        return;
    }

    // negative value
    if (value < 0)
    {
        tmpValue = -tmpValue;
        value    = -value;
        pBuf[pos++] = '-';
    }

    // calculate the required length of the buffer
    do {
        pos++;
        tmpValue /= base;
    } while(tmpValue > 0);


    if (pos > len)
    {
        // the len parameter is invalid.
        return;
    }

    pBuf[pos] = '\0';

    do {
        pBuf[--pos] = pAscii[value % base];
        value /= base;
    } while(value > 0);

    return;

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

    int i = 0;
    uint32_t offset = 240;
    uint8_t b[E_WRITE_LEN];
    int32_t len = 0;
    uint16_t psz = 0;

    init_ssp();
    init_uart();

    UART_SendString(UART_DEV, (uint8_t*)"Flash example\r\n");

    for (i = 0; i < E_WRITE_LEN; i++) {
        b[i] = (uint8_t)(i);
    }

    if (!flash_init()) {
    	UART_SendString(UART_DEV, (uint8_t*)"Flash: Failed to initialize\r\n");
        return 1;
    }
    psz = flash_getPageSize();
    intToString(psz, buf, 10, 10);
    UART_SendString(UART_DEV,(uint8_t*)"Flash: Page size = ");
    UART_SendString(UART_DEV, buf);
    UART_SendString(UART_DEV, (uint8_t*)"\r\n");

    len = flash_write(b, offset, E_WRITE_LEN);

    if (len != E_WRITE_LEN) {
    	UART_SendString(UART_DEV, (uint8_t*)"Flash: Failed to write data\r\n");
        return 1;
    }

    UART_SendString(UART_DEV, (uint8_t*)"Flash: Data written\r\n");
    Timer0_Wait(2000);

    memset(b, 0, E_WRITE_LEN);

    UART_SendString(UART_DEV, (uint8_t*)"Flash: Reading\r\n");
    len = flash_read(b, offset, E_WRITE_LEN);

    if (len != E_WRITE_LEN) {
    	UART_SendString(UART_DEV, (uint8_t*)"Flash: Failed to read all data\r\n");
        return 1;
    }

    UART_SendString(UART_DEV, (uint8_t*)"Flash: Verifing\r\n");
    for (i = 0; i < E_WRITE_LEN; i++) {
        if (b[i] != (uint8_t)(i)) {

        	UART_SendString(UART_DEV, (uint8_t*)"Flash: Invalid data\r\n");

            return 1;
        }
    }

    UART_SendString(UART_DEV, (uint8_t*) "Flash: OK\r\n");

    return 0;

}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
