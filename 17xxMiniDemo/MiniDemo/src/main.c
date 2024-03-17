/*************************************************************************************
 *
 * @Description:
 * Program przykładowy - odpowiednik "Hello World" dla systemów wbudowanych
 * Rekomendujemy wkopiowywanie do niniejszego projektu nowych funkcjonalności
 *
 *
 * Program przykładowy wykorzystuje Timer #0 i Timer #1 do "mrugania" diodami
 * Dioda P0.8 jest zapalona i gaszona, a czas pomiędzy tymi zdarzeniami
 * odmierzany jest przez Timer #0.
 * Program aktywnie oczekuje na upłynięcie odmierzanego czasu (1s)
 *
 * Druga z diod P0.9 jest gaszona i zapalana w takt przerwań generowanych
 * przez timer #1, z okresem 500 ms i wypełnieniem 20%.
 * Procedura obsługi przerwań zdefiniowana jest w innym pliku (irq/irq_handler.c)
 * Sama procedura MUSI być oznaczona dla kompilatora jako procedura obsługi
 * przerwania odpowiedniego typu. W przykładzie jest to przerwanie wektoryzowane.
 * Odpowiednia deklaracja znajduje się w pliku (irq/irq_handler.h)
 *
 * Prócz "mrugania" diodami program wypisuje na konsoli powitalny tekst.
 *
 * @Authors: Michał Morawski,
 *           Daniel Arendt,
 *           Przemysław Ignaciuk,
 *           Marcin Kwapisz
 *
 * @Change log:
 *           2016.12.01: Wersja oryginalna.
 *           2023.03.16: Drobne, acz istotne zmiany związane z nowszą wersją kompilatora
 *
 ******************************************************************************/

#include "system_LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_clkpwr.h"
#include <stdio.h>

#include "Common_Definitions.h"
// Uwaga! Zmienne trzeba dostosować do posiadanej plytki (Rev A, Rev B, Rev D)

#define USE_MESSAGES
//To makro musi być wyłączone gdy program NIE jest uruchamiany z debuggerem
//w przeciwnym razie zawiesi się na funkcji printf!!!




/*************************************************************
 * @brief: Obsługa błędów krytycznych
 * @param file[in]: komunikat, typowo nazwa pliku: makro __FILE__, choć niekoniecznie
 * @param line[in]: nr linii typowo uzyskiwany makrem __LINE__
 * @side effects: program ugrzęźnie w tej procedurze (nigdy z niej nie wyjdzie)
 */
void
blad_krytyczny (char* file, uint32_t line)
{
  // Tu można wypisać coś mądrego
  printf ("Blad krytyczny (%s) w linii %lu\r\n", file, line);

  // I zatrzymaj program (ALE NIE PRZERWANIA!)
  while (TRUE)
    {
    }
}


/*************************************************************
 * @brief: Inicjalizacja wybranego pinu GPIO
 * @param port[in]: numer portu
 * @param line[in]: nr bitu
 * @return: none
 * @side effects: none
 * @description: Procedura inicjuje port GPIO wskazany w parametrach ustawiając
 *               go na wyjście i ustawiając na nim stan niski
 */
static void
init_LED_port_and_pin (uint8_t port, uint8_t pin)
{
  PINSEL_CFG_Type PinCfg;

  PinCfg.Funcnum = 0;	// GPIO
  PinCfg.OpenDrain = 0;
  PinCfg.Pinmode = 0;
  PinCfg.Pinnum = pin;
  PinCfg.Portnum = port;
  PINSEL_ConfigPin (&PinCfg);
  // Kierunek out (bo to dioda)
  GPIO_SetDir (port, _BIT(pin), 1);
  // a teraz ją wyłącz
  GPIO_ClearValue (port, _BIT(pin));
}

/*************************************************************
 * @brief: Inicjalizacja portów GPIO, do których są przyłączone diody LED
 * @return: none
 * @side effects: none
 * @description: Procedura inicjuje 6 portów GPIO, do których przyłączone są w sumie
 *               dwie diody RGB.
 *               Dioda R-Alt (obecna tylko na płytce RevD) jest wyłączona (stan wysoki)
 */
static void
init_all_LEDs (void)
{
  // zainicjowane są wszystkie 3, choć de cacto nie ma takiej potrzeby
  init_LED_port_and_pin (LED_RED_PORT, LED_RED_PIN);
  init_LED_port_and_pin (LED_BLUE_PORT, LED_BLUE_PIN);
  init_LED_port_and_pin (LED_GREEN_PORT, LED_GREEN_PIN);
  // dodatkowe działają tylko na płytce REV D
  init_LED_port_and_pin (LED_RED_PORT_ALT, LED_RED_PIN_ALT);
  init_LED_port_and_pin (LED_BLUE_PORT_ALT, LED_BLUE_PIN_ALT);
  init_LED_port_and_pin (LED_GREEN_PORT_ALT, LED_GREEN_PIN_ALT);
  GPIO_SetValue (LED_RED_PORT_ALT, _BIT(LED_RED_PIN_ALT)); // wyłącz czerwoną diodę
}

/*************************************************************
 * @brief: Inicjalizacja zegara systemowego "Systick"
 * @return: none
 * @side effects: none
 * @description: Procedura inicjuje przerwania SysTick co 100ms
 */
static void
init_Systick (void)
{
  if (SysTick_Config (SystemCoreClock / 10)) // czyli 10 razy na sekundę
    {
      blad_krytyczny (__FILE__, __LINE__);
    }
}

/*************************************************************
 * @brief: Inicjalizacja Timera T1
 * @return: none
 * @side effects: none
 * @description: Inicjalizacja  zegara T1 w taki sposób, żeby generował przerwania co 200ms i co 2s.
 */
static void
init_Timer (void)
{
  TIM_TIMERCFG_Type Config;
  TIM_MATCHCFG_Type Match_Cfg;

  Config.PrescaleOption = TIM_PRESCALE_USVAL;
  Config.PrescaleValue = 1000; // czyli 1 milisekunda

  //najpierw włączyć zasilanie
  CLKPWR_SetPCLKDiv (CLKPWR_PCLKSEL_TIMER1, CLKPWR_PCLKSEL_CCLK_DIV_1);
  // Ustawić timer.
  TIM_Cmd (LPC_TIM1, DISABLE); // to w zasadzie jest niepotrzebne, ponieważ po włączeniu zasilania timer jest nieczynny,
  TIM_Init (LPC_TIM1, TIM_TIMER_MODE, &Config);

  Match_Cfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
  Match_Cfg.IntOnMatch = TRUE;
  Match_Cfg.ResetOnMatch = FALSE;
  Match_Cfg.StopOnMatch = FALSE;
  Match_Cfg.MatchChannel = 0;
  Match_Cfg.MatchValue = 200; // Czyli 200 ms.
  TIM_ConfigMatch (LPC_TIM1, &Match_Cfg);
  Match_Cfg.ResetOnMatch = TRUE;
  Match_Cfg.MatchChannel = 1;
  Match_Cfg.MatchValue = 2000; // czyli 2 s.
  TIM_ConfigMatch (LPC_TIM1, &Match_Cfg);
  TIM_Cmd (LPC_TIM1, ENABLE);
  // I odblokować przerwania w VIC
  NVIC_EnableIRQ (TIMER1_IRQn);
}

int
main (void)
{
#ifdef USE_MESSAGES
  printf ("\n\n\n\n");
  printf ("\n*********************************************************");
  printf ("\n*");
  printf ("\n* Systemy Wbudowane");
  printf ("\n* Wydzial FTIMS");
  printf ("\n* Moj pierwszy program");
  printf ("\n*");
  printf ("\n*********************************************************");
#endif

  // inicjalizacja peryferiów
  init_Systick ();
  init_all_LEDs ();
  init_Timer ();

  Bool dioda_swieci = FALSE;

  while (TRUE)
    {
      if (dioda_swieci)
	{
	  GPIO_ClearValue (LED_GREEN_PORT, _BIT(LED_GREEN_PIN));
#ifdef USE_MESSAGES
	  printf ("\nDioda nie swieci...");
#endif
	  GPIO_ClearValue (LED_GREEN_PORT_ALT, _BIT(LED_GREEN_PIN_ALT)); // a ta dioda się świeci!
	}
      else
	{
	  GPIO_SetValue (LED_GREEN_PORT, _BIT(LED_GREEN_PIN));
#ifdef USE_MESSAGES
	  printf ("\nA teraz swieci...");
#endif
	  GPIO_SetValue (LED_GREEN_PORT_ALT, _BIT(LED_GREEN_PIN_ALT)); // a ta dioda zgasła!!!
	}
      dioda_swieci = !dioda_swieci;
      Timer0_Wait (1000); // 1 sekunda
      // Należy zwrócić uwagę na rozsynchronizowanie się diód (tej z main, i tej z przerwania)
      // Jaka jest tego przyczyna?
    }
}


