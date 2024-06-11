/*****************************************************************************
 *   A demo example using several of the peripherals on the base board
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************/
#include "lpc17xx_adc.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_rtc.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_timer.h"


#include "acc.h"
#include "eeprom.h"
#include "joystick.h"
#include "light.h"
#include "oled.h"
#include "temp.h"


#define NOTE_PIN_HIGH() GPIO_SetValue(0, 1<<26);
#define NOTE_PIN_LOW()  GPIO_ClearValue(0, 1<<26);


static void playNote(uint32_t note, uint32_t durationMs);
static uint32_t getNote(uint8_t ch);
static uint32_t getDuration(uint8_t ch);
static uint32_t getPause(uint8_t ch);
static void playSong(uint8_t *song);
static void init_speaker(void);
static void init_ssp(void);
static void init_i2c(void);
static void init_adc(void);
static void setUpRTC(void);

static void setNewTime(uint8_t * time);
static void setTime(void);
static void setAlarm(void);
static void handle_joystick(void);

static void saveTimeToEEPROM(const RTC_TIME_Type *rtc);
static void readTimeFromEEPROM(RTC_TIME_Type *rtc);
static void saveAlarmToEEPROM(uint8_t hours,  uint8_t minutes, uint8_t seconds);
static void readAlarmFromEEPROM(void);

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

/* Zmienne do przechowywania czasu budzika */
static uint8_t alarm_godziny = 4;
static uint8_t alarm_minuty = 20;
static uint8_t alarm_sekundy = 4;

static uint8_t edit_mode = 0;

static uint32_t msTicks; /* counts 1ms timeTicks */

static oled_color_t background = OLED_COLOR_BLACK;
static oled_color_t foreground = OLED_COLOR_WHITE;
static uint8_t print_value;

/*
	OLED							+
	Zegar RTC						+
	Timer							+
	Głośnik/dźwięk					+0.5
	GPIO joystick					+
	I2c i/f do 5					+
	Akcelerometr gasi alarm (i2c) 	+
	Termometr (czujnik)				+0.5
	Eeprom do zapisu alarmu			+
	Dodatkowo:
	Luxometr						+

*/

int main (void) {
	uint32_t lux_value;
	uint8_t prev_mode = 0;
	uint8_t prev_light = 0;
	uint8_t light_change = 0;

	int32_t temperature;
	const int8_t MARGIN = 5;

    init_i2c();
    init_ssp();
    init_adc();
    init_speaker();

    joystick_init();
    acc_init();
    oled_init();

    eeprom_init();

    setUpRTC();

    light_init();
    light_setRange(LIGHT_RANGE_4000);
    light_setMode(LIGHT_MODE_D2);
    light_enable();

    readAlarmFromEEPROM();


    if (SysTick_Config(SystemCoreClock / 1000)) {
		while(1) {
			/* error */
		}
	}

	temp_init(&getTicks);

	setTime();

    oled_clearScreen(OLED_COLOR_BLACK);

    while (1) {
    	lux_value = light_read();
    	if (lux_value < (uint32_t) 20) {
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
			if (!(msTicks % (uint8_t) 10))
			{
				temperature = temp_read();
			}


			char temperatura[20];
			print_value = snprintf(temperatura, sizeof(temperatura), "Temp: %d.%d", (int8_t) (temperature / 10), (int8_t) (temperature % 10));
			oled_putString(1, 20, (const uint8_t *) temperatura, foreground, background);

			char czas[20];
			print_value = snprintf(czas, sizeof(czas), "Czas: %02d:%02d:%02d", (uint8_t ) RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR), (uint8_t) RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE), (uint8_t) RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND));
			oled_putString(1, 1, (const uint8_t *) czas, foreground, background);
			oled_putString(1, 40, (const uint8_t *) "C - ustaw czas", foreground, background);
			print_value = snprintf(czas, sizeof(czas), "Alarm: %02d:%02d:%02d", alarm_godziny, alarm_minuty, alarm_sekundy);
			oled_putString(1, 50, (const uint8_t *) czas, foreground, background);

		} else {
			// W menu ustawiania budzika
			oled_putString(1, 1, (const uint8_t *) "Ustaw czas:", foreground, background);
			char czas[20];
			print_value = snprintf(czas, sizeof(czas), "Czas: %02d:%02d:%02d", (uint8_t) RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR), (uint8_t) RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE), (uint8_t) RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND));
			oled_putString(1, 10, (const uint8_t *) czas, foreground, background);
			print_value = snprintf(czas, sizeof(czas), "Alarm: %02d:%02d:%02d", alarm_godziny, alarm_minuty, alarm_sekundy);
			oled_putString(1, 20, (const uint8_t *) czas, foreground, background);

			oled_putString(1, 40, (const uint8_t *) "L - alarm", foreground, background);
			oled_putString(1, 50, (const uint8_t *) "R - zegar", foreground, background);
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
				xdiff = (int8_t) (xoff - x);
				ydiff = (int8_t) (yoff - y);
				zdiff = (int8_t) (zoff - z);
//				btn1 = ((GPIO_ReadValue(0) >> 4) & 0x01);
				if(!((-MARGIN < xdiff) && (xdiff < MARGIN) && (-MARGIN < ydiff) && (ydiff < MARGIN) && (-MARGIN < zdiff) && (zdiff < MARGIN))) {
					edit_mode = 0;
					prev_mode = 1;
					break;
				}
			}
		}
		handle_joystick(); // Obsługa joysticka
		Timer0_us_Wait(50);
    }

}

static void setNewTime(uint8_t * time) {
	uint8_t set_time = 0;
	uint8_t edit_position = 0;
	uint8_t joystick_state;
	char buff[20];
	char position;
	// uint8_t g = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR);
	// uint8_t m = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE);
	// uint8_t s = RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND);
	uint8_t g = time[0];
	uint8_t m = time[1];
	uint8_t s = time[2];
	while (!set_time) {
		joystick_state = joystick_read();
		if (joystick_state & JOYSTICK_UP) { // Joystick w górę
			if (!edit_position) {
				g = (uint8_t)((g + (uint8_t)(1)) % (uint8_t)(24));
			}
			else if (edit_position == (uint8_t)(1)) {
				m = (uint8_t)((m + (uint8_t)(1)) % (uint8_t)(60));
			}
			else {
				s = (uint8_t)((s + (uint8_t)(1)) % (uint8_t)(60));
			}
		} else if (joystick_state & JOYSTICK_DOWN) { // Joystick w dół
			if (!edit_position) {
				g = (g == (uint8_t)(0)) ? (uint8_t)(23) : (uint8_t) (g - (uint8_t)(1));
			}
			else if (edit_position == (uint8_t)(1)) {
				m = (m == (uint8_t)(0)) ? (uint8_t)(59) : (uint8_t) (m - (uint8_t)(1));
			}
			else {
				s = (s == (uint8_t)(0)) ? (uint8_t)(59) : (uint8_t) (s - (uint8_t)(1));
			}
		} else if (joystick_state & JOYSTICK_RIGHT) { // Joystick w prawo
			edit_position = (uint8_t)((edit_position + (uint8_t)(1)) % (uint8_t)(3));
		} else if (joystick_state & JOYSTICK_LEFT) { // Joystick w prawo
			edit_position = (uint8_t)((edit_position - (uint8_t)(1)) % (uint8_t)(3));
		} else {}

		if (!edit_position) {
			position = 'g';
		}
		else if (edit_position == (uint8_t)(1)) {
			position = 'm';
		}
		else {
			position = 's';
		}

		print_value = snprintf(buff, sizeof(buff), "Ustaw: %02d:%02d:%02d", g, m, s);
		oled_putString(1, 20, (const uint8_t *) buff, foreground, background);
		oled_putChar(69, 30, (uint8_t) position, foreground, background);
		set_time = joystick_read() & JOYSTICK_CENTER;
		Timer0_Wait(50);
	}
	time[0] = g;
	time[1] = m;
	time[2] = s;
	oled_clearScreen(background);
}

static void setTime(void) {
	uint8_t time[] = {RTC_GetTime(LPC_RTC, RTC_TIMETYPE_HOUR), RTC_GetTime(LPC_RTC, RTC_TIMETYPE_MINUTE), RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND)};
	oled_clearScreen(background);
	oled_putString(1, 1, (const uint8_t *) "Czas:", foreground, background);
	oled_putString(1, 30, (const uint8_t *) "Zmieniasz: ", foreground, background);
	setNewTime(time);
	RTC_TIME_Type RTCFullTime;
	RTCFullTime.HOUR = time[0];
	RTCFullTime.MIN = time[1];
	RTCFullTime.SEC = time[2];
	RTC_SetFullTime(LPC_RTC, &RTCFullTime);
	saveTimeToEEPROM(&RTCFullTime);

}

static void setAlarm(void) {
	uint8_t time[]= {alarm_godziny, alarm_minuty, alarm_sekundy};
	oled_clearScreen(background);
	oled_putString(1, 1, (const uint8_t *) "Alarm:", foreground, background);
	oled_putString(1, 30, (const uint8_t *) "Zmieniasz: ", foreground, background);
	setNewTime(time);
	alarm_godziny = time[0];
	alarm_minuty = time[1];
	alarm_sekundy = time[2];
	saveAlarmToEEPROM(alarm_godziny, alarm_minuty, alarm_sekundy);
}

/* Funkcja do obsługi joysticka */
static void handle_joystick(void) {
    //uint8_t joystick_state = GPIO_ReadValue(1);
	uint8_t joystick_state;
	uint8_t read_position =  joystick_read();
	if (!read_position){
		return;
	}
	joystick_state = read_position;
	if(joystick_state == JOYSTICK_CENTER){
		edit_mode ^= 1;
	}
	if (edit_mode) {
		if (joystick_state & JOYSTICK_RIGHT) { // Joystick w prawo ustawnianie czasu
			setTime();
		}
		if (joystick_state & JOYSTICK_LEFT) { // Joystick w lewo ustawianie alarmu
			setAlarm();
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

static void readAlarmFromEEPROM(void) {
	uint8_t buf[3];
	eeprom_read(buf, 3, 3);
	alarm_godziny = buf[0];
	alarm_minuty = buf[1];
	alarm_sekundy = buf[2];
}

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

static void init_speaker(void) {
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
    /* <---- Speaker ------ */
}

static void playNote(uint32_t note, uint32_t durationMs) {

    uint32_t t = 0;

    if (note > (uint8_t) 0) {

        while (t < (durationMs*(uint32_t)1000)) {
            NOTE_PIN_HIGH();
            Timer0_us_Wait(note / (uint32_t) 2);
            //delay32Us(0, note / 2);

            NOTE_PIN_LOW();
            Timer0_us_Wait(note / (uint32_t) 2);
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
	uint32_t result = 0;
    if ((ch >= (uint8_t) 'A') && (ch <= (uint8_t) 'G')) {
        result = notes[ch - (uint8_t) 'A'];
	}
    if ((ch >= (uint8_t) 'a') && (ch <= (uint8_t) 'g')) {
        result = notes[ch - (uint8_t) 'a' + (uint8_t) 7];
	}
    return result;
}

static uint32_t getDuration(uint8_t ch) {\
	uint32_t result;
    if ((ch < (uint8_t) '0') || (ch > (uint8_t) '9')) {
        result = 400;
	} else {
		result = (ch - (uint32_t) '0') * (uint32_t)  200;
	}
    /* number of ms */

    return result;
}

static uint32_t getPause(uint8_t ch) {
	uint32_t result;
    switch (ch) {
    case '+':
        result = 0;
		break;
    case ',':
        result = 5;
		break;
    case '.':
        result = 20;
		break;
    case '_':
        result = 30;
		break;
    default:
        result = 5;
		break;
    }
	return result;
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
        if (*song == '\0') {
            break;
		}
        dur  = getDuration(*song++);
        if (*song == '\0') {
            break;
		}
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
