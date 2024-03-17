/**************************************************************************
 * @Description:
 * Obsługa przerwań
 */

#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "Common_Definitions.h"

/*************************************************************
 * @brief: Obsługa przerwania od Timera#1
 * @Description: Obsługiwane są przypadki MR0 i MR1
 *               W jednym przypadku diody się zapalają, w drugim gasną
 */
void
TIMER1_IRQHandler (void)
{
  if (TIM_GetIntStatus (LPC_TIM1, TIM_MR0_INT))
    {
      GPIO_ClearValue (LED_RED_PORT, _BIT(LED_RED_PIN));             // wyłącz
      GPIO_ClearValue (LED_BLUE_PORT_ALT, _BIT(LED_BLUE_PIN_ALT));   // włącz
                                                                     // ta sama instrukcja,
                                                                     // ale inaczej włączona dioda
                                                                     // sprzętowo!
      TIM_ClearIntPending (LPC_TIM1, TIM_MR0_INT);
    }
  if (TIM_GetIntStatus (LPC_TIM1, TIM_MR1_INT))
    {
      GPIO_SetValue (LED_RED_PORT, _BIT(LED_RED_PIN));              // włącz
      GPIO_SetValue (LED_BLUE_PORT_ALT, _BIT(LED_BLUE_PIN_ALT));    // wyłącz
								    // ta sama instrukcja!!!
      TIM_ClearIntPending (LPC_TIM1, TIM_MR1_INT);
    }
}

////////////////////////////////////////////////////
// Przerwania typu SysTick
volatile uint32_t Relative_Time_Tick; // niezainicjowane. Nie ma gwarancji, że zacznie od zera.

/*************************************************************
 * @brief: Obsługa przerwania od Timera SysTick
 * @Description: Procedura nie robi nic poza zwiększaniem zmiennej Relative_Time_Tick
 */
void
SysTick_Handler (void)
{
  Relative_Time_Tick++; // Zegar systemowy.
}
