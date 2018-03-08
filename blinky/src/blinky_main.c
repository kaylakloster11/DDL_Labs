/****************************************************************************
 *   $Id:: blinky_main.c 4785 2010-09-03 22:39:27Z nxp21346                        $
 *   Project: LED flashing / ISP test program
 *
 *   Description:
 *     This file contains the main routine for the blinky code sample
 *     which flashes an LED on the LPCXpresso board and also increments an
 *     LED display on the Embedded Artists base board. This project
 *     implements CRP and is useful for testing bootloaders.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/

#include "driver_config.h"
#include "target_config.h"

#include "timer32.h"
#include "gpio.h"

#include "NUM_Converter.h"
/* Main Program */

extern int fibonacci(int x);


int main(void) {

	/* Initialize 32-bit timer 0. TIMER_INTERVAL is defined as 10 mS */

	init_timer32(0,TIME_INTERVAL);

	/*Initialize GPIO (sets up clock) */
	GPIOInit();

	/* Set LED port pin to output */
		GPIOSetDir(LED_PORT_R, LED_BIT_R, 1);
		GPIOSetDir(LED_PORT_R, LED_BIT_B, 1);

	int fib_num;
	int n = 1;
    //int * morse_bin;
    int loop_val;

    enable_timer32(0);

	// Enter an infinite loop, just incrementing a counter
	volatile static int loop = 0;
	while (1) {
		 for(n = 1; n<= 20; n++){
			fib_num = fibonacci(n);
			NUM_Converter(fib_num);

			for(int i=3;i>=0; i--){
				get_Morse(hex_num[i]);
				loop_val = morse[4];
				if((loop_val == 1) | (loop_val == 0)){
					loop_val = 5;
				}
				else if(loop_val == 5){
					loop_val == 1;
				}
				for(int j = 0; j < loop_val; j++){
					if(morse[j] == 1){
						timer32_0_counter = 0;
						while((timer32_0_counter%(LED_TOGGLE_TICKS/COUNT_MAX)) < ((LED_TOGGLE_TICKS/COUNT_MAX)/2)){
							GPIOSetValue(LED_PORT_R, LED_BIT_R, LED_ON);
							__WFI();
						}
					}
					else if(morse[j] == 0){
						timer32_0_counter = 0;
						while((timer32_0_counter%(LED_TOGGLE_TICKS/COUNT_MAX)) < ((LED_TOGGLE_TICKS/COUNT_MAX)/6)){
							GPIOSetValue(LED_PORT_R, LED_BIT_R, LED_ON);
							__WFI();
						}
					}
					timer32_0_counter = 0;
					while((timer32_0_counter%(LED_TOGGLE_TICKS/COUNT_MAX)) < ((LED_TOGGLE_TICKS/COUNT_MAX)/6)){
						GPIOSetValue(LED_PORT_R, LED_BIT_R, LED_OFF);
						__WFI();
					}
				}
				timer32_0_counter = 0;
				while((timer32_0_counter%(LED_TOGGLE_TICKS/COUNT_MAX))<((LED_TOGGLE_TICKS/COUNT_MAX)/2)){
					GPIOSetValue(LED_PORT_R, LED_BIT_R, LED_OFF);
					GPIOSetValue(LED_PORT_B, LED_BIT_B, LED_ON);
				}
				GPIOSetValue(LED_PORT_B, LED_BIT_B, LED_OFF);
			}
		}
		loop++;
	  }
   }
