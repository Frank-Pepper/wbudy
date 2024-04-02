/*****************************************************************************
 *   Reading data from UART1 and writing to UART2 (and vice versa)
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

#include "uart2.h"

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

	uartCfg.Baud_rate = 115200;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;

	UART_Init(UART_DEV, &uartCfg);

	UART_TxCmd(UART_DEV, ENABLE);

}

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

int main (void) {

	uint8_t data = 0;
    uint8_t uart1Read = 1;
    uint32_t recvd = 0;
    uint32_t len = 0;


    init_i2c();
    init_uart();
    uart2_init(115200, CHANNEL_A);

    UART_SendString(UART_DEV, (uint8_t*)"Flash example\r\n");

    if (uart1Read) {
    	UART_SendString(UART_DEV,(uint8_t*)"\r\nReading data from this UART\r\n");
        uart2_sendString((uint8_t*)"\r\nWriting data from UART1 to this UART\r\n");
    }
    else {
        uart2_sendString((uint8_t*)"\r\nReading data from this UART\r\n");
        UART_SendString(UART_DEV,(uint8_t*)"\r\nWriting data from UART2 to this UART\r\n");
    }

    while (1) {
        if (uart1Read) {
            len = UART_Receive(UART_DEV, &data,
            		1, NONE_BLOCKING);
        }
        else {
            len = uart2_receive(&data, 1, FALSE);
        }

        if (len > 0) {
            if (uart1Read) {
                uart2_send(&data, 1);
            }
            else {
                UART_Send(UART_DEV, &data, 1, NONE_BLOCKING);
            }

            recvd++;
        }

        if (recvd >= 10) {
            uart1Read = !uart1Read;

            if (uart1Read) {
            	UART_SendString(UART_DEV,(uint8_t*)"\r\nReading data from this UART\r\n");
                uart2_sendString((uint8_t*)"\r\nWriting data from UART1 to this UART\r\n");
            }
            else {
                uart2_sendString((uint8_t*)"\r\nReading data from this UART\r\n");
                UART_SendString(UART_DEV,(uint8_t*)"\r\nWriting data from UART2 to this UART\r\n");
            }
            recvd = 0;
        }

        Timer0_Wait(50);
    }


    while(1);
}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
