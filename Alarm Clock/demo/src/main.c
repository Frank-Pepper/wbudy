#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_rtc.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_gpio.h"

#include "joystick.h"
#include "stdio.h"
#include "oled.h"
#include "time.h"
#include "acc.h"
#include "temp.h"


/* Funkcje inicjalizujące */
static void init_i2c(void);
static void init_ssp(void);
static void setUpRTC(void);
static void oled_setup(void);
static void init_Timer(void);
void handle_joystick(void);

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

void SysTick_Handler(void) {
    msTicks++;
}

uint32_t getTicks(void) {
    return msTicks;
}

int main(void) {
    init_i2c();
    init_ssp();
    setUpRTC();
    oled_setup();
    init_Timer();

    joystick_init();
    acc_init();
    if (SysTick_Config(SystemCoreClock / 1000)) {
        while(1); // error
    }

    temp_init(&getTicks);


    while(1) {
    	if (edit_mode != prev_mode) {
    		oled_clearScreen(OLED_COLOR_BLACK);
    		prev_mode = edit_mode;
    	}
        if (!edit_mode) {
            // W normalnym trybie wyświetlania czasu
            oled_putString(1, 1, (uint8_t *) "Czas:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            char czas[9];
            snprintf(czas, sizeof(czas), "%02d:%02d:%02d", godziny, minuty, sekundy);
            oled_putString(1, 20, (uint8_t *) czas, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            oled_putString(1, 40, (uint8_t *) "R - ustaw budzik", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            snprintf(czas, sizeof(czas), "%02d:%02d:%02d", alarm_godziny, alarm_minuty, alarm_sekundy);
            oled_putString(1, 50, (uint8_t *) czas, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

            // Read and display temperature
            temp = temp_read();
            char temperatura[10];
            snprintf(temperatura, sizeof(temperatura), "Temp: %d", temp / 10);
            oled_putString(1, 60, (uint8_t *) temperatura, OLED_COLOR_WHITE, OLED_COLOR_BLACK);

        } else {
            // W menu ustawiania budzika
            oled_putString(1, 1, (uint8_t *) "Ustaw budzik:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            char alarm[9];
            snprintf(alarm, sizeof(alarm), "%02d:%02d:%02d", alarm_godziny, alarm_minuty, alarm_sekundy);
            oled_putString(1, 20, (uint8_t *) alarm, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            oled_putString(1, 30, (uint8_t *) "U - zwieksz", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            oled_putString(1, 40, (uint8_t *) "D - zmniejsz", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
            oled_putString(1, 50, (uint8_t *) "R - zatwierdz", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
        }
        // Sprawdzenie, czy aktualny czas odpowiada czasowi budzika
        if (godziny == alarm_godziny && minuty == alarm_minuty && sekundy == alarm_sekundy) {
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
        	while(1){
				oled_putString(1, 40, (uint8_t *) "Wakey, wakey!", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
				oled_clearScreen(OLED_COLOR_BLACK);
				acc_read(&xoff, &yoff, &zoff);
				xdiff = xoff - x;
				ydiff = yoff - y;
				zdiff = zoff - z;
//				btn1 = ((GPIO_ReadValue(0) >> 4) & 0x01);
				if(!(-margin < xdiff && xdiff < margin && -margin < ydiff && ydiff < margin && -margin < zdiff && zdiff < margin)) {
					edit_mode = 0;
					prev_mode = 1;
					break;
				}
        	}
}

        handle_joystick(); // Obsługa joysticka
    }
}

/* Inicjalizacja interfejsu I2C */
static void init_i2c(void) {
    PINSEL_CFG_Type PinCfg;
    PinCfg.Funcnum = 2;
    PinCfg.Pinnum = 10;
    PinCfg.Portnum = 0;
    PINSEL_ConfigPin(&PinCfg);
    PinCfg.Pinnum = 11;
    PINSEL_ConfigPin(&PinCfg);

    I2C_Init(LPC_I2C2, 100000);
    I2C_Cmd(LPC_I2C2, ENABLE);
}

/* Inicjalizacja interfejsu SSP */
static void init_ssp(void) {
    SSP_CFG_Type SSP_ConfigStruct;
    PINSEL_CFG_Type PinCfg;

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
    SSP_Init(LPC_SSP1, &SSP_ConfigStruct);
    SSP_Cmd(LPC_SSP1, ENABLE);
}

/* Inicjalizacja RTC */
static void setUpRTC(void) {
    RTC_Init(LPC_RTC);
    RTC_ResetClockTickCounter(LPC_RTC);
    RTC_Cmd(LPC_RTC, ENABLE);

    RTC_TIME_Type RTCFullTime;
    RTCFullTime.HOUR = 4;
    RTCFullTime.MIN = 20;
    RTCFullTime.SEC = 0;
    RTC_SetFullTime(LPC_RTC, &RTCFullTime);
    RTC_GetFullTime(LPC_RTC, &RTCFullTime);
    godziny = RTCFullTime.HOUR;
    minuty = RTCFullTime.MIN;
    sekundy = RTCFullTime.SEC;
}

/* Inicjalizacja wyświetlacza OLED */
static void oled_setup(void) {
    oled_init();
    oled_clearScreen(OLED_COLOR_BLACK);
}

/* Inicjalizacja Timera */
static void init_Timer(void) {
    TIM_TIMERCFG_Type Config;
    TIM_MATCHCFG_Type Match_Cfg;

    Config.PrescaleOption = TIM_PRESCALE_USVAL;
    Config.PrescaleValue = 1000; // Prescaler 1, co sekundę

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &Config);

    Match_Cfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    Match_Cfg.IntOnMatch = TRUE;
    Match_Cfg.ResetOnMatch = TRUE;
    Match_Cfg.StopOnMatch = FALSE;
    Match_Cfg.MatchChannel = 0;
    Match_Cfg.MatchValue = 1000; // 1 sekunda
    TIM_ConfigMatch(LPC_TIM0, &Match_Cfg);

    TIM_Cmd(LPC_TIM0, ENABLE);
    NVIC_EnableIRQ(TIMER0_IRQn); // Włączenie przerwań od Timera 0
}

/* Obsługa przerwania od Timera 0 */
void TIMER0_IRQHandler(void) {
    if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)) {
        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
        sekundy++; // Zwiększenie sekund

        // Aktualizacja minut i godzin w zależności od upływu czasu
        if (sekundy >= 60) {
            sekundy = 0;
            minuty++;
            if (minuty >= 60) {
                minuty = 0;
                godziny++;
                if (godziny >= 24) {
                    godziny = 0;
                }
            }
        }
    }
}

/* Funkcja do obsługi joysticka */
void handle_joystick(void) {
    //uint8_t joystick_state = GPIO_ReadValue(1);
	uint8_t read_position =  joystick_read();
	if (read_position == 0){
		return;
	}
	joystick_state = read_position;
	if(joystick_state == JOYSTICK_CENTER){
		edit_mode ^= 1;
	}
	if (edit_mode) {
		if (joystick_state & JOYSTICK_UP) { // Joystick w górę
			if (menu_state == 0) alarm_godziny = (alarm_godziny + 1) % 24;
			else if (menu_state == 1) alarm_minuty = (alarm_minuty + 1) % 60;
			else if (menu_state == 2) alarm_sekundy = (alarm_sekundy + 1) % 60;
//			else if (menu_state == 3) {
//				// W menu ustawiania budzika, joystick w górę zwiększa wartość aktualnie ustawianego pola
//				if (alarm_godziny < 23) alarm_godziny++;
//				else if (alarm_minuty < 59) alarm_minuty++;
//				else if (alarm_sekundy < 59) alarm_sekundy++;
//			}
		} else if (joystick_state & JOYSTICK_DOWN) { // Joystick w dół
			if (menu_state == 0) alarm_godziny = alarm_godziny == 0 ? 23 : alarm_godziny - 1;
			else if (menu_state == 1) alarm_minuty = alarm_minuty == 0 ? 59 : alarm_minuty - 1;
			else if (menu_state == 2) alarm_sekundy = alarm_sekundy == 0 ? 59 : alarm_sekundy - 1;
		} else if (joystick_state & JOYSTICK_RIGHT) { // Joystick w prawo
			menu_state = (menu_state + 1) % 3;
//			if (menu_state < 3) menu_state = (menu_state + 1) % 3;
//			else {
//				// W menu ustawiania budzika, joystick w prawo zatwierdza ustawienia i wraca do normalnego trybu wyświetlania czasu
//				menu_state = 0;
//			}
		} else if (joystick_state & JOYSTICK_LEFT) { // Joystick w prawo
			menu_state = (menu_state - 1) % 3;
//			if (menu_state > 0) menu_state = (menu_state - 1) % 3;
//			else {
//				// W menu ustawiania budzika, joystick w prawo zatwierdza ustawienia i wraca do normalnego trybu wyświetlania czasu
//				menu_state = 3;
//			}
		}
    }
}
