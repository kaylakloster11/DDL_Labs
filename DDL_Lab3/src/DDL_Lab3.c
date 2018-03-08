/*
===============================================================================
 Name        : DDL_Lab3.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC11xx.h"
#endif

#include <cr_section_macros.h>


	volatile int DUTY_CYCLE = 0x01;
	int TimerInterval;
	volatile int gpio2_counter;
	volatile int timer32_0_counter;
	volatile int timer32_1_counter;
	volatile int last_period_count = 0;
	volatile int period = 10000;


	void GPIOInit(void){
	    LPC_GPIO0->DIR |= (0x1<<7);
	    LPC_GPIO0->DATA |= (0x1<<7);
	    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);
	    LPC_GPIO2->IS &= ~(0x1<<0);
	    LPC_GPIO2->IBE &= ~(0x1<<0);
	    LPC_GPIO2->IEV &= ~(0x1<<0);

	    LPC_GPIO2->IE |= (0x1<<0);

	    gpio2_counter = 0;
	    NVIC_EnableIRQ(EINT2_IRQn);
	    NVIC_SetPriority(EINT2_IRQn, 0);
	}

    void TIMERInit(void){
    	/* Some of the I/O pins need to be carefully planned if
			you use below module because JTAG and TIMER CAP/MAT pins are muxed. */
    	int TimerInterval = (SystemCoreClock/12000);
    	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);
    	LPC_IOCON->PIO1_5 &= ~0x07;	/*  Timer0_32 I/O config */
    	LPC_IOCON->PIO1_5 |= 0x02;	/* Timer0_32 CAP0 */
    	LPC_IOCON->PIO1_6 &= ~0x07;
    	LPC_IOCON->PIO1_6 |= 0x02;	/* Timer0_32 MAT0 */
    	LPC_IOCON->PIO1_7 &= ~0x07;
    	LPC_IOCON->PIO1_7 |= 0x02;	/* Timer0_32 MAT1 */
    	LPC_IOCON->PIO0_1 &= ~0x07;
    	LPC_IOCON->PIO0_1 |= 0x02;	/* Timer0_32 MAT2 */

    	timer32_0_counter = 0;
    	timer32_1_counter = 0;

		LPC_TMR32B0->MR0 = TimerInterval;
		//Capture zero on rising edge, interrupt enable
		LPC_TMR32B0->CCR = (0x1<<0)|(0x1<<2);
		LPC_TMR32B0->MCR = 3;
		NVIC_EnableIRQ(TIMER_32_0_IRQn);
		NVIC_SetPriority(TIMER_32_0_IRQn, 0);
		LPC_TMR32B0->TCR = 1;
    }

    void PIOINT2_IRQHandler(void){
    	uint32_t regVal;

    		  regVal = ( LPC_GPIO2->MIS & (0x1<<0));
    		  if ( regVal )
    		  {
    			  LPC_GPIO2->IC |= (0x1<<0);
    			  LPC_GPIO0->DATA &= ~(0x1<<7);
    			  gpio2_counter++;
    			 period = timer32_0_counter - last_period_count;
    			 last_period_count = timer32_0_counter;
    		  }
    		  return;
    }

 void TIMER32_0_IRQHandler(void){
    	if ( LPC_TMR32B0->IR & 0x01 )
    	  {
    		LPC_TMR32B0->IR = (0x01);				/* clear interrupt flag */
    		timer32_0_counter++;
    		timer32_1_counter++;
    		if(timer32_1_counter == 100000){
    			DUTY_CYCLE ^= (0x01<<1);
    			timer32_1_counter = 0;
    		}
    		if((timer32_0_counter - last_period_count)> ((DUTY_CYCLE*period)/4)){
    			 LPC_GPIO0->DATA |= (0x1<<7);
    		}
    	  }
    	  return;
    }

int main(void) {

	TIMERInit(); //Initialize Timer and Generate a 1ms interrupt
	GPIOInit();  //Initialize GPIO ports for both Interrupts and LED control

    /* Infinite looping */
    while(1);

    return 0 ;
}
