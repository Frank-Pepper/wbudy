/*****************************************************************************
 *   A demo example using several of the peripherals on the base board
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/


#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"

#include "lpc17xx_rtc.h"
#include "lpc17xx_clkpwr.h"
#include "time.h"
#include "temp.h"
#include "light.h"
#include "eeprom.h"

#include "joystick.h"
#include "acc.h"
#include "oled.h"

#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);


static void playNote(uint32_t note, uint32_t durationMs);
static uint32_t getNote(uint8_t ch);
static uint32_t getDuration(uint8_t ch);
static uint32_t getPause(uint8_t ch);
static void playSong(uint8_t *song);
static void init_ssp(void);
static void init_i2c(void);
static void init_adc(void);
static void setUpRTC(void);
static void handle_joystick(void);

static void saveTimeToEEPROM(const RTC_TIME_Type *rtc);
static void readTimeFromEEPROM(RTC_TIME_Type *rtc);
static void saveAlarmToEEPROM(uint8_t hours,  uint8_t minutes, uint8_t seconds);
static void setTime();

void SysTick_Handler(void);
uint32_t getTicks(void);


static uint32_t notes[] = {
        2272, // A - 440 Hz
        2024, // B - 494 Hz
        3816, // C - 262 Hz
        3401, // D - 294 Hz
        3030, // E - 330 Hz
        2865, // F - 349 Hz
        2551, // G - 392 Hz
        1136, // a - 880 Hz
        1012, // b - 988 Hz
        1912, // c - 523 Hz
        1703, // d - 587 Hz
        1517, // e - 659 Hz
        1432, // f - 698 Hz
        1275, // g - 784 Hz
};

static uint8_t * song = (uint8_t*)"C2.C2,D4,C4,F4,E8,";
        //(uint8_t*)"C2.C2,D4,C4,F4,E8,C2.C2,D4,C4,G4,F8,C2.C2,c4,A4,F4,E4,D4,A2.A2,H4,F4,G4,F8,";
        //"D4,B4,B4,A4,A4,G4,E4,D4.D2,E4,E4,A4,F4,D8.D4,d4,d4,c4,c4,B4,G4,E4.E2,F4,F4,A4,A4,G8,";


/* Zmienne do przechowywania aktualnego czasu */
volatile uint8_t godziny = 0;
volatile uint8_t minuty = 0;
volatile uint8_t sekundy = 0;

/* Zmienne do przechowywania czasu budzika */
volatile uint8_t alarm_godziny = 4;
volatile uint8_t alarm_minuty = 20;
volatile uint8_t alarm_sekundy = 4;

/* Zmienna do przechowywania stanu menu */
volatile uint8_t menu_state = 0;
volatile uint8_t joystick_state = 0;
volatile uint8_t edit_mode = 0;
volatile uint8_t prev_mode = 0;
volatile uint8_t btn1 = 1;

volatile int32_t temp;

const uint8_t margin = 5;

volatile uint32_t msTicks; /* counts 1ms timeTicks */

uint32_t jasno;

oled_color_t background = OLED_COLOR_BLACK;
oled_color_t foreground = OLED_COLOR_WHITE;
uint8_t prev_light = 0;
uint8_t light_change = 0;

int main (void) {


    init_i2c();
    init_ssp();
    init_adc();

    joystick_init();
    acc_init();
    oled_init();

    eeprom_init();

    setUpRTC();

    light_init();
    light_enable();


    if (SysTick_Config(SystemCoreClock / 1000)) {
		while(1); // error
	}

	temp_init(&getTicks);



    /* ---- Speaker ------> */

    GPIO_SetDir(2, 1<<0, 1);
    GPIO_SetDir(2, 1<<1, 1);

    GPIO_SetDir(0, 1<<27, 1);
    GPIO_SetDir(0, 1<<28, 1);
    GPIO_SetDir(2, 1<<13, 1);
    GPIO_SetDir(0, 1<<26, 1);

    GPIO_ClearValue(0, 1<<27); //LM4811-clk
    GPIO_ClearValue(0, 1<<28); //LM4811-up/dn
    GPIO_ClearValue(2, 1<<13); //LM4811-shutdn

    uint8_t btn1 = ((GPIO_ReadValue(0) >> 4) & 0x01);

	setTime();
    /* <---- Speaker ------ */

    oled_clearScreen(OLED_COLOR_BLACK);


    while (1) {
    	jasno = light_read();
    	if (jasno < 200) {
    		background = OLED_COLOR_WHITE;
    		foreground = OLED_COLOR_BLACK;
    		light_change = 1;
    	} else {
    		background = OLED_COLOR_BLACK;
    		foreground = OLED_COLOR_WHITE;
    		light_change = 0;
    	}
    	if (light_change != prev_light) {
    		oled_clearScreen(background);
    		prev_light = light_change;
    	}
    	if (edit_mode != prev_mode) {
			oled_clearScreen(background);
			prev_mode = edit_mode;
		}
		if (!edit_mode) {
			// W normalnym trybie wyświetlania czasu
			// Read and display temperature
			if ((msTicks % 10) == 0)
			{
				temp = temp_read();
			}

			godziny = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR);
			minuty = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE);
			sekundy = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND);

			char temperatura[20];
			snprintf(temperatura, sizeof(temperatura), "Temp: %d.%d", temp / 10, temp % 10);
			oled_putString(1, 20, (uint8_t *) temperatura, foreground, background);

			char czas[20];
			snprintf(czas, sizeof(czas), "Czas: %02d:%02d:%02d", godziny, minuty, sekundy);
			oled_putString(1, 1, (uint8_t *) czas, foreground, background);
			oled_putString(1, 40, (const uint8_t *) "R - ustaw budzik", foreground, background);
			snprintf(czas, sizeof(czas), "%02d:%02d:%02d", alarm_godziny, alarm_minuty, alarm_sekundy);
			oled_putString(1, 50, (uint8_t *) czas, foreground, background);

		} else {
			// W menu ustawiania budzika
			oled_putString(1, 1, (const uint8_t *) "Ustaw budzik:", foreground, background);
			char alarm[9];
			snprintf(alarm, sizeof(alarm), "%02d:%02d:%02d", alarm_godziny, alarm_minuty, alarm_sekundy);
			oled_putString(1, 20, (uint8_t *) alarm, foreground, background);
			oled_putString(1, 30, (const uint8_t *) "U - zwieksz", foreground, background);
			oled_putString(1, 40, (const uint8_t *) "D - zmniejsz", foreground, background);
			oled_putString(1, 50, (const uint8_t *) "R - zatwierdz", foreground, background);
		}
		// Sprawdzenie, czy aktualny czas odpowiada czasowi budzika
		if ((RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR) == alarm_godziny) && (RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE) == alarm_minuty) && (RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND) == alarm_sekundy)) {
			// Wywołanie alarmu
			// accelerometr coordynaty
			int8_t x;
			int8_t y;
			int8_t z;
			int8_t xoff;
			int8_t yoff;
			int8_t zoff;
			int8_t xdiff;
			int8_t ydiff;
			int8_t zdiff;
			acc_read(&x, &y, &z);
			oled_clearScreen(background);
			oled_putString(1, 40, (const uint8_t *) "Wakey, wakey!", foreground, background);
			playSong(song);
			while(1){
				oled_putString(1, 40, (const uint8_t *) "Wakey, wakey!", foreground, background);
				oled_clearScreen(background);
				acc_read(&xoff, &yoff, &zoff);
				xdiff = xoff - x;
				ydiff = yoff - y;
				zdiff = zoff - z;
//				btn1 = ((GPIO_ReadValue(0) >> 4) & 0x01);
				if(!((-margin < xdiff) && (xdiff < margin) && (-margin < ydiff) && (ydiff < margin) && (-margin < zdiff) && (zdiff < margin))) {
					edit_mode = 0;
					prev_mode = 1;
					break;
				}
			}
		}
		handle_joystick(); // Obsługa joysticka
//        Timer0_Wait(1);
    }

}

static void setTime() {
	uint8_t set_time = 0;
	menu_state = 0;
	uint8_t joystick_state;
	uint8_t buff[20];
	uint8_t g = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR);
	uint8_t m = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE);
	uint8_t s = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND);
	oled_clearScreen(background);
	while (!set_time) {
		joystick_state = joystick_read();
		if (joystick_state & JOYSTICK_UP) { // Joystick w górę
			if (!menu_state) {
				g = (uint8_t)((g + (uint8_t)(1)) % (uint8_t)(24));
			}
			else if (menu_state == (uint8_t)(1)) {
				m = (uint8_t)((m + (uint8_t)(1)) % (uint8_t)(60));
			}
			else if (menu_state == (uint8_t)(2)) {
				s = (uint8_t)((s + (uint8_t)(1)) % (uint8_t)(60));
			}
		} else if (joystick_state & JOYSTICK_DOWN) { // Joystick w dół
			if (menu_state == (uint8_t)(0)) {
				g = (g == (uint8_t)(0)) ? (uint8_t)(23) : (uint8_t) (g - (uint8_t)(1));
			}
			else if (menu_state == (uint8_t)(1)) {
				m = (m == (uint8_t)(0)) ? (uint8_t)(59) : (uint8_t) (m - (uint8_t)(1));
			}
			else if (menu_state == (uint8_t)(2)) {
				s = (s == (uint8_t)(0)) ? (uint8_t)(59) : (uint8_t) (s - (uint8_t)(1));
			}
		} else if (joystick_state & JOYSTICK_RIGHT) { // Joystick w prawo
			menu_state = (uint8_t)((menu_state + (uint8_t)(1)) % (uint8_t)(3));
		} else if (joystick_state & JOYSTICK_LEFT) { // Joystick w prawo
			menu_state = (uint8_t)((menu_state - (uint8_t)(1)) % (uint8_t)(3));
		}
		snprintf(buff, sizeof(buff), "Ustaw: %02d:%02d:%02d", g, m, s);
		oled_putString(1, 20, (uint8_t *) buff, foreground, background);
		set_time = joystick_read() & JOYSTICK_CENTER;
		Timer0_Wait(1);
	}
	RTC_TIME_Type RTCFullTime;
	RTCFullTime.HOUR = g;
	RTCFullTime.MIN = m;
	RTCFullTime.SEC = s;
	saveTimeToEEPROM(&RTCFullTime);
	RTC_SetFullTime(LPC_RTC, &RTCFullTime);
}

/* Funkcja do obsługi joysticka */
static void handle_joystick(void) {
    //uint8_t joystick_state = GPIO_ReadValue(1);
	uint8_t read_position =  joystick_read();
	if (!read_position){
		return;
	}
	joystick_state = read_position;
	if(joystick_state == JOYSTICK_CENTER){
		edit_mode ^= 1;
	}
	if (edit_mode) {
		if (joystick_state & JOYSTICK_UP) { // Joystick w górę
			if (!menu_state) {
                alarm_godziny = (uint8_t)((alarm_godziny + (uint8_t)(1)) % (uint8_t)(24));
            }
			else if (menu_state == (uint8_t)(1)) {
                alarm_minuty = (uint8_t)((alarm_minuty + (uint8_t)(1)) % (uint8_t)(60));
            }
			else if (menu_state == (uint8_t)(2)) {
                alarm_sekundy = (uint8_t)((alarm_sekundy + (uint8_t)(1)) % (uint8_t)(60));
            }
		} else if (joystick_state & JOYSTICK_DOWN) { // Joystick w dół
			if (menu_state == (uint8_t)(0)) {
                alarm_godziny = (alarm_godziny == (uint8_t)(0)) ? (uint8_t)(23) : (uint8_t) (alarm_godziny - (uint8_t)(1));
            }
			else if (menu_state == (uint8_t)(1)) {
                alarm_minuty = (alarm_minuty == (uint8_t)(0)) ? (uint8_t)(59) : (uint8_t) (alarm_minuty - (uint8_t)(1));
            }
			else if (menu_state == (uint8_t)(2)) {
                alarm_sekundy = (alarm_sekundy == (uint8_t)(0)) ? (uint8_t)(59) : (uint8_t) (alarm_sekundy - (uint8_t)(1));
            }
		} else if (joystick_state & JOYSTICK_RIGHT) { // Joystick w prawo
			menu_state = (uint8_t)((menu_state + (uint8_t)(1)) % (uint8_t)(3));
//			if (menu_state < 3) menu_state = (menu_state + 1) % 3;
//			else {
//				// W menu ustawiania budzika, joystick w prawo zatwierdza ustawienia i wraca do normalnego trybu wyświetlania czasu
//				menu_state = 0;
//			}
		} else if (joystick_state & JOYSTICK_LEFT) { // Joystick w prawo
			menu_state = (uint8_t)((menu_state - (uint8_t)(1)) % (uint8_t)(3));
//			if (menu_state > 0) menu_state = (menu_state - 1) % 3;
//			else {
//				// W menu ustawiania budzika, joystick w prawo zatwierdza ustawienia i wraca do normalnego trybu wyświetlania czasu
//				menu_state = 3;
//			}
		}
    }
}

/* Inicjalizacja RTC */
static void setUpRTC(void) {

    RTC_Init(LPC_RTC);
    RTC_ResetClockTickCounter(LPC_RTC);
    RTC_Cmd(LPC_RTC, ENABLE);

    RTC_TIME_Type RTCFullTime;
    readTimeFromEEPROM(&RTCFullTime);
//    RTCFullTime.HOUR = 4;
//    RTCFullTime.MIN = 20;
//    RTCFullTime.SEC = 0;
    RTC_SetFullTime(LPC_RTC, &RTCFullTime);
    RTC_GetFullTime(LPC_RTC, &RTCFullTime);

    saveTimeToEEPROM(&RTCFullTime);
}

static void saveTimeToEEPROM(const RTC_TIME_Type *rtc) {
	uint8_t buf[3];
	buf[0] = rtc->SEC;
	buf[1] = rtc->MIN;
	buf[2] = rtc->HOUR;
	eeprom_write(buf, 0, 3);
}

static void readTimeFromEEPROM(RTC_TIME_Type *rtc) {
	uint8_t buf[3];
	eeprom_read(buf, 0, 3);
	rtc->SEC = buf[0];
	rtc->MIN = buf[1];
	rtc->HOUR = buf[2];
}

static void saveAlarmToEEPROM(uint8_t hours,  uint8_t minutes, uint8_t seconds) {
	uint8_t buf[3];
	buf[0] = hours;
	buf[1] = minutes;
	buf[2] = seconds;
	eeprom_write(buf, 3, 3);
}

//void readAlarmFromEEPROM(uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
//
//}

static void init_ssp(void) {
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

static void init_i2c(void) {
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

static void init_adc(void) {
	PINSEL_CFG_Type PinCfg;

	/*
	 * Init ADC pin connect
	 * AD0.0 on P0.23
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 23;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);

	/* Configuration for ADC :
	 * 	Frequency at 0.2Mhz
	 *  ADC channel 0, no Interrupt
	 */
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC,ADC_CHANNEL_0,DISABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_0,ENABLE);

}

static void playNote(uint32_t note, uint32_t durationMs) {

    uint32_t t = 0;

    if (note > 0) {

        while (t < (durationMs*1000)) {
            NOTE_PIN_HIGH();
            Timer0_us_Wait(note / 2);
            //delay32Us(0, note / 2);

            NOTE_PIN_LOW();
            Timer0_us_Wait(note / 2);
            //delay32Us(0, note / 2);

            t += note;
        }

    }
    else {
    	Timer0_Wait(durationMs);
        //delay32Ms(0, durationMs);
    }
}

static uint32_t getNote(uint8_t ch) {
    if (ch >= 'A' && ch <= 'G')
        return notes[ch - 'A'];

    if (ch >= 'a' && ch <= 'g')
        return notes[ch - 'a' + 7];

    return 0;
}

static uint32_t getDuration(uint8_t ch) {
    if (ch < '0' || ch > '9')
        return 400;

    /* number of ms */

    return (ch - '0') * 200;
}

static uint32_t getPause(uint8_t ch) {
    switch (ch) {
    case '+':
        return 0;
    case ',':
        return 5;
    case '.':
        return 20;
    case '_':
        return 30;
    default:
        return 5;
    }
}

static void playSong(uint8_t *song) {
    uint32_t note = 0;
    uint32_t dur  = 0;
    uint32_t pause = 0;

    /*
     * A song is a collection of tones where each tone is
     * a note, duration and pause, e.g.
     *
     * "E2,F4,"
     */

    while(*song != '\0') {
        note = getNote(*song++);
        if (*song == '\0')
            break;
        dur  = getDuration(*song++);
        if (*song == '\0')
            break;
        pause = getPause(*song++);

        playNote(note, dur);
        //delay32Ms(0, pause);
        Timer0_Wait(pause);

    }
}

void SysTick_Handler(void) {
    msTicks++;
}

uint32_t getTicks(void) {
    return msTicks;
}

void check_failed(uint8_t *file, uint32_t line) {
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while(1);
}
