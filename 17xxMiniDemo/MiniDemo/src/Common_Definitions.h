#ifndef __COMMON_DEFINITIONS_H__
#define __COMMON_DEFINITIONS_H__

#include "lpc_types.h"

/*************************************************************
 * Definicje portów, do których podłączone są diody
 */
//#define USE_LPCXPRESSO_REV_D

#define LED_RED_PORT_EXP	 (2)
#define LED_RED_PIN_EXP	 	 (0)
#define LED_GREEN_PORT_EXP	 (2)
#define LED_GREEN_PIN_EXP	 (1)
#define LED_BLUE_PORT_EXP	 (0)
#define LED_BLUE_PIN_EXP	 (26)
// te diody są zamontowane wyłącznie na płytce REV D
#define LED_RED_PORT_ALT 	 (0)
#define LED_RED_PIN_ALT	 	 (22)
#define LED_GREEN_PORT_ALT	 (3)
#define LED_GREEN_PIN_ALT	 (25)
#define LED_BLUE_PORT_ALT	 (3)
#define LED_BLUE_PIN_ALT	 (26)

#ifdef USE_LPCXPRESSO_REV_D
	#define LED_RED_PORT		LED_RED_PORT_ALT
	#define LED_RED_PIN			LED_RED_PIN_ALT
	#define LED_GREEN_PORT		LED_GREEN_PORT_ALT
	#define LED_GREEN_PIN		LED_GREEN_PIN_ALT
	#define LED_BLUE_PORT		LED_BLUE_PORT_ALT
	#define LED_BLUE_PIN		LED_BLUE_PIN_ALT
#else
	#define LED_RED_PORT		LED_RED_PORT_EXP
	#define LED_RED_PIN			LED_RED_PIN_EXP
	#define LED_GREEN_PORT		LED_GREEN_PORT_EXP
	#define LED_GREEN_PIN		LED_GREEN_PIN_EXP
	#define LED_BLUE_PORT		LED_BLUE_PORT_EXP
	#define LED_BLUE_PIN		LED_BLUE_PIN_EXP
#endif //USE_LPCXPRESSO_REV_D


#endif //__COMMON_DEFINITIONS_H__
