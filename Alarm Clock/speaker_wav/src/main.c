/*****************************************************************************
 *   This example is is playing a WAV file
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"



extern const unsigned char sound_8k[];
extern int sound_sz;

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


int main (void)
{
	PINSEL_CFG_Type PinCfg;

    uint32_t cnt = 0;
    uint32_t off = 0;
    uint32_t sampleRate = 0;
    uint32_t delay = 0;

    GPIO_SetDir(2, 1<<0, 1);
    GPIO_SetDir(2, 1<<1, 1);

    GPIO_SetDir(0, 1<<27, 1);
    GPIO_SetDir(0, 1<<28, 1);
    GPIO_SetDir(2, 1<<13, 1);
    GPIO_SetDir(0, 1<<26, 1);

    GPIO_ClearValue(0, 1<<27); //LM4811-clk
    GPIO_ClearValue(0, 1<<28); //LM4811-up/dn
    GPIO_ClearValue(2, 1<<13); //LM4811-shutdn

	/*
	 * Init DAC pin connect
	 * AOUT on P0.26
	 */
	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 26;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	/* init DAC structure to default
	 * Maximum	current is 700 uA
	 * First value to AOUT is 0
	 */
	DAC_Init(LPC_DAC);

    init_uart();


    /* ChunkID */
    if (sound_8k[cnt] != 'R' && sound_8k[cnt+1] != 'I' &&
        sound_8k[cnt+2] != 'F' && sound_8k[cnt+3] != 'F')
    {
    	UART_SendString(UART_DEV, (uint8_t*)"Wrong format (RIFF)\r\n");
        return 0;
    }
    cnt+=4;

    /* skip chunk size*/
    cnt += 4;

    /* Format */
    if (sound_8k[cnt] != 'W' && sound_8k[cnt+1] != 'A' &&
        sound_8k[cnt+2] != 'V' && sound_8k[cnt+3] != 'E')
    {
    	UART_SendString(UART_DEV, (uint8_t*)"Wrong format (WAVE)\r\n");
        return 0;
    }
    cnt+=4;

    /* SubChunk1ID */
    if (sound_8k[cnt] != 'f' && sound_8k[cnt+1] != 'm' &&
        sound_8k[cnt+2] != 't' && sound_8k[cnt+3] != ' ')
    {
    	UART_SendString(UART_DEV, (uint8_t*)"Missing fmt\r\n");
        return 0;
    }
    cnt+=4;

    /* skip chunk size, audio format, num channels */
    cnt+= 8;

    sampleRate = (sound_8k[cnt] | (sound_8k[cnt+1] << 8) |
            (sound_8k[cnt+2] << 16) | (sound_8k[cnt+3] << 24));

    if (sampleRate != 8000) {
    	UART_SendString(UART_DEV, (uint8_t*)"Only 8kHz supported\r\n");
        return 0;
    }

    delay = 1000000 / sampleRate;

    cnt+=4;

    /* skip byte rate, align, bits per sample */
    cnt += 8;

    /* SubChunk2ID */
    if (sound_8k[cnt] != 'd' && sound_8k[cnt+1] != 'a' &&
        sound_8k[cnt+2] != 't' && sound_8k[cnt+3] != 'a')
    {
    	UART_SendString(UART_DEV, (uint8_t*)"Missing data\r\n");
        return 0;
    }
    cnt += 4;

    /* skip chunk size */
    cnt += 4;

    off = cnt;

    while(1)
    {

        cnt = off;
        while(cnt++ < sound_sz)
        {
        	DAC_UpdateValue ( LPC_DAC,(uint32_t)(sound_8k[cnt]));
        	Timer0_us_Wait(delay);
        }

    }

    return 0 ;
}

void check_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
