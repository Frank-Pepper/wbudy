#include "lpc17xx_pinsel.h"
#include "lpc17xx_i2c.h"
#include "lpc17xx_ssp.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_rtc.h"
#include "lpc17xx_clkpwr.h"
#include "stdio.h"
#include "oled.h"
#include "time.h"

/* Funkcje inicjalizujące */
static void init_i2c(void);
static void init_ssp(void);
static void setUpRTC(void);
static void oled_setup(void);
static void init_Timer(void);
void TIMER0_IRQHandler(void);

/* Zmienne do przechowywania aktualnego czasu */
volatile uint8_t godziny = 0;
volatile uint8_t minuty = 0;
volatile uint8_t sekundy = 0;

//#define USE_MESSAGES

int main(void) {
    init_i2c();
    init_ssp();
    oled_setup();
    init_Timer();
    char czas[9];
    oled_putString(1, 1, (uint8_t *) "Czas:", OLED_COLOR_WHITE, OLED_COLOR_BLACK);
    setUpRTC();
#ifdef USE_MESSAGES
    time_t seconds;
        struct tm *timeStruct;
        seconds = time(NULL);
        timeStruct = localtime(&seconds);
        timeStruct->tm_hour += 2;
        godziny = timeStruct->tm_hour;
        minuty = timeStruct->tm_min;
        sekundy = timeStruct->tm_sec;

//        printf("Current time : %d:%d:%d\n", timeStruct->tm_hour, timeStruct->tm_min, timeStruct->tm_sec);
#endif

    while(1) {
        // Aktualizacja czasu na ekranie OLED
        snprintf(czas, sizeof(czas), "%02d:%02d:%02d", godziny, minuty, sekundy);
//        printf("value of a_static: %s\n", czas);
        TIMER0_IRQHandler();
        uint8_t * temp = (uint8_t *) czas;
        oled_putString(1, 20, temp, OLED_COLOR_WHITE, OLED_COLOR_BLACK);
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