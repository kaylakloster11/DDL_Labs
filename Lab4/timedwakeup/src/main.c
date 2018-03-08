/****************************************************************************
 *   $Id:: invokeisp_main.c 4816 2010-09-07 17:24:10Z nxp21346                        $
 *   Project: NXP LPC11xx timedwakeup example
 *
 *   Description:
 *     Code sample demonstrating entry into sleep and deep sleep
 *     power save modes.
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

//#undef DEBUG
//#define ENABLE_CLKOUT

// Uncomment below to flash an LED. The LED consumes current.
#define FLASH_LED

#define LPCXPRESSO_LPC1343_BOARD

#include "driver_config.h"
#include "target_config.h"

#include "rom_drivers.h"

#include "timer16.h"

#include "wakeupdefs.h"

// Configuration for normal operation
// If other peripherals are used, they need to be added to the _RUN macro
#define BF_PDRUNCFG_RUN    (BF_PDCFG_RESERVEDMSK   \
                            & ~(  BF_PDCFG_IRC     \
                                | BF_PDCFG_FLASH   \
                                | BF_PDCFG_WDTOSC))

// If other peripherals are used, they need to be added to the _RUN macro
#define BF_SYSAHBCLKCTRL_RUN       (BF_SYSAHBCLKCTRL_SLEEP  \
                                  | BF_SYSAHBCLKCTRL_GPIO   \
								  | BF_SYSAHBCLKCTRL_FLASHREG \
                                  | BF_SYSAHBCLKCTRL_ROM    \
                                  | BF_SYSAHBCLKCTRL_IOCON)

#define MainClockFrequency 12000000
#define WDTClockFrequency 9000 /* estimate used during debug mode */

// Approximate go to sleep+wake time in # of clock cycles
#define WUTIME_CLOCKS           60

// When changing any of the values below, check that none of the
// 16-bit timer code overflows
#define PROCDURATION_MS			50
#define  LEDDURATION_MS         2000
#define WAKEDURATION_MS			(PROCDURATION_MS + LEDDURATION_MS)
#define SLEEPDURATION_MS        2000
#define WDOMEASUREDURATION_MS   WAKEDURATION_MS

unsigned int command[5], result[5];             //command and result arrays
const ROM ** rom = (const ROM **) 0x1FFF1FF8;
int currentClockSpeed = 48;

int fibonacci(int n)
{
    int c;

    if (n == 1 || n == 2)
        return 1;

    c = fibonacci(n-2) + fibonacci(n-1);

    return c;
}

int InitFromPowerAPIDemo()
{
	if((*rom)->pPWRD == (void *)0xFFFFFFFF)
	    {
	    	// This LPC does not have the power API
	    	while(1);
	    }

	    LPC_SYSCON->SYSAHBCLKCTRL = (0xFFFFFFFF & (~(1<<15))); //enable all clocks except the WDT

	    /* user must select correct PLL input source before calling power/pll routines */
	    LPC_SYSCON->PDRUNCFG &= ~(1<<5);            //power-up the system oscillator
	    int i;
	    for (i = 0; i != 24000; i++);               //wait for it to stabilize
	    LPC_SYSCON->SYSPLLCLKSEL = 0x01;            //system PLL source is the system oscillator
	    LPC_SYSCON->SYSPLLCLKUEN = 0x00;            //update the system PLL source...
	    LPC_SYSCON->SYSPLLCLKUEN = 0x01;            //...
	    LPC_SYSCON->MAINCLKSEL = 0x01;              //main clock source is the PLL input
	    LPC_SYSCON->MAINCLKUEN = 0x00;              //update the main clock source...
	    LPC_SYSCON->MAINCLKUEN = 0x01;              //...

	    LPC_SYSCON->CLKOUTCLKSEL = 0x03;            //CLKOUT = main clock
	    LPC_SYSCON->CLKOUTUEN = 0x01;               //update CLKOUT selection...
	    LPC_SYSCON->CLKOUTUEN = 0x00;               //...
	    LPC_SYSCON->CLKOUTUEN = 0x01;               //...
	    LPC_SYSCON->CLKOUTDIV = 10;                 //generate ouptut @ 1/10 rate

	    LPC_IOCON->PIO0_1 &= ~0x07;                 //select CLKOUT @ PIO0_1...
	    LPC_IOCON->PIO0_1 |= 1;                     //...

		//configure CT16B1_MAT0 as a system/10 clock out (P0_21)
	//	LPC_IOCON->PIO0_21 = (LPC_IOCON->PIO0_21 & ~0x7) | 1;	//select CT16B1_MAT0 @ P0_21
	    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<8);           	//enable CT16B1 clock
		LPC_TMR16B1->TCR	= 0x00;						//stop the timer
		LPC_TMR16B1->TCR	= 0x02;						//reset the timer
		LPC_TMR16B1->TCR	= 0x00;						//release the reset
		LPC_TMR16B1->CTCR	= 0x00;						//count on peripheral clock rising edges only
	  	LPC_TMR16B1->PR		= 0;						//max rate (no prescaler)
		LPC_TMR16B1->MR0 	= 10/2-1;					//match output = 1/10 timer's clock
	  	LPC_TMR16B1->MCR	= 1<<1;						//reset on MR0
	  	LPC_TMR16B1->EMR	= 3<<4;						//toggle MAT0
		LPC_TMR16B1->TCR	= 0x01;						//let the timer run

	    /* user must select the correct operating frequency before setting up the PLL   */
	    /* or the voltage regulator and the flash interface setup might not be able     */
	    /* to support application running at higher frequencies                         */
	    command[0] = 48;                            //system freq 48 MHz
	//    command[1] = PARAM_DEFAULT;                 //specify system power to default
	//    command[1] = PARAM_CPU_EXEC;                //specify system power for cpu performance run
	//    command[1] = PARAM_EFFICIENCY;              //specify system power for efficiency
	    command[1] = PARAM_LOW_CURRENT;             //specify system power for low active current
	    (*rom)->pPWRD->set_power(command,result);   //set system power
}

void EnterDeepSleep(void)
{
    LPC_TMR16B0->EMR = BF_TIMER_EMR_SETOUT0; // set timer to drive P0_8 high at match

#ifndef DEBUG
    // Shut down clocks to almost everything
    LPC_SYSCON->SYSAHBCLKCTRL = BF_SYSAHBCLKCTRL_SLEEP;

    SCB->SCR |= BF_SCR_SLEEPDEEP; // Set SLEEPDEEP bit so MCU will enter DeepSleep mode on __WFI();

    // Switch main clock to low-speed WDO
    LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_SEL_WDOSC;
    LPC_SYSCON->MAINCLKUEN = 0;
    LPC_SYSCON->MAINCLKUEN = 1; // toggle to enable
    LPC_SYSCON->MAINCLKUEN = 0;
#else
    // In debug mode, we are only going to sleep mode, not deep sleep
    // We must disable the SysTick interrupt because it will wake us from
    // Sleep mode
    SysTick->CTRL = 0; // enable counter, interrupts, select processor clock
#endif

    // Preload clock selection for quick switch back to IRC on wakeup
    LPC_SYSCON->MAINCLKSEL = MAINCLKSEL_SEL_IRCOSC;

    LPC_TMR16B0->TCR = BF_TIMER_TCR_RUN; // start sleep timer

    __WFI();                            // Enter deep sleep mode (sleep mode in DEBUG)
}

void WAKEUP_IRQHandler(void)
{
    LPC_SYSCON->MAINCLKUEN = 1;         // Restore main clock to IRC 12 MHz

    LPC_TMR16B0->EMR = 0;					// Clear match bit on timer

    LPC_SYSCON->STARTRSRP0CLR       =   BF_STARTLOGIC_P0_8; // Clear pending bit on start logic

    SCB->SCR &= ~BF_SCR_SLEEPDEEP; // Clear SLEEPDEEP bit so MCU will enter Sleep mode on __WFI();


    // Restore clocks to chip modules
    LPC_SYSCON->SYSAHBCLKCTRL = BF_SYSAHBCLKCTRL_RUN;

#ifdef DEBUG
    // We disable systick to sleep, must re-enable it (in debug mode only)
    SysTick->CTRL = 7; // enable counter, interrupts, select processor clock
#endif
}

void SysTick_Handler(void)
{
}

void Wait1mS(uint32_t input)
{
	uint32_t ii = (input * currentClockSpeed)/18;
    SysTick->VAL = 0; // clear counter

    while(ii > 0)
    {
        // Wait for systick counter to count down and reset
        while(!(SysTick->CTRL & BF_SYSTICK_COUNTFLAG))
        	__WFI();

        ii--;
    }
}

void InitDeepSleep(void)
{
#ifdef ENABLE_CLKOUT
    /* Output the Clk onto the CLKOUT Pin PIO0_1 to monitor the freq on a scope */
    LPC_IOCON->PIO0_1       = 0xC1;
    /* Select the MAIN clock as the clock out selection since it's driving the core */
    LPC_SYSCON->CLKOUTCLKSEL = 3;
    LPC_SYSCON->CLKOUTDIV = 10;
    LPC_SYSCON->CLKOUTUEN = 0;
    LPC_SYSCON->CLKOUTUEN = 1;

#endif /* ENABLE_CLKOUT */

    // Set up Systick timer for 1 mS timeouts
    // Used for general timing when awake and to calibrate the WDT
    SysTick->LOAD = (MainClockFrequency / 1000)-1;
    SysTick->VAL = 0; // reload counter
    SysTick->CTRL = 7; // enable counter, interrupts, select processor clock

    LPC_SYSCON->PDAWAKECFG =       // Configure PDAWAKECFG to restore PDRUNCFG on wake up
            LPC_SYSCON->PDRUNCFG;

    LPC_SYSCON->PDSLEEPCFG = BF_PDSLEEPCFG_WDT; // Configure deep sleep with WDT oscillator

    LPC_TMR16B0->TCR = BF_TIMER_TCR_RESET; // reset timer

    // The following lines initializing PR and MR0 are at risk for overflow
    // if the timing and frequency parameters are changed because they are
    // using a 16-bit timer.
#ifndef DEBUG
    LPC_TMR16B0->PR = 0;
    LPC_TMR16B0->MR0 = (SLEEPDURATION_MS*MeasureWDO()/WDOMEASUREDURATION_MS - WUTIME_CLOCKS - 1);
#else
    LPC_TMR16B0->PR = (MainClockFrequency / WDTClockFrequency) -1;
    LPC_TMR16B0->MR0 = SLEEPDURATION_MS*WDTClockFrequency/1000;
#endif

    LPC_TMR16B0->MCR = BF_TIMER_MCR_MATCHSTOP0 | BF_TIMER_MCR_MATCHRESET0;

    LPC_IOCON->PIO0_8 = (LPC_IOCON->PIO0_8 & ~0x3F) | 0x2; // Set IOCON register on P0.8 to match function

    /* Configure Wakeup I/O */
    /* Specify the start logic to allow the chip to be waken up using PIO0_8 */
    LPC_SYSCON->STARTAPRP0          |=  BF_STARTLOGIC_P0_8; // Rising edge
    LPC_SYSCON->STARTRSRP0CLR       =   BF_STARTLOGIC_P0_8; // Clear pending bit
    LPC_SYSCON->STARTERP0           |=  BF_STARTLOGIC_P0_8; // Enable Start Logic
    NVIC_EnableIRQ(WAKEUP8_IRQn);
}

uint32_t MeasureWDO(void)
{
    uint32_t start,end;
    static uint32_t WDO_clocks;

    if(!WDO_clocks)
    {
        // Configure Watchdog Oscillator
        LPC_SYSCON->WDTOSCCTRL = (0x1<<5) | (0x1F<<0);

        LPC_SYSCON->SYSAHBCLKCTRL |= BF_SYSAHBCLKCTRL_WDT;  // Turn on clock to WDT register block

        // Now enable clock to WDT counter
#if defined(DEBUG)
        LPC_SYSCON->WDTCLKSEL = WDTCLKSEL_SEL_MAINCLK;
#else
        LPC_SYSCON->WDTCLKSEL = WDTCLKSEL_SEL_WDOSC;
#endif
        LPC_SYSCON->WDTCLKUEN = 0;  // Arm WDT clock selection update
        LPC_SYSCON->WDTCLKUEN = 1;  // Update WDT clock selection
        LPC_SYSCON->WDTCLKUEN = 0;  // Arm WDT clock selection update
        LPC_SYSCON->WDTCLKDIV = 1;  // WDT clock divide by 1

        LPC_WDT->TC = 0xFFFFFF;   // big value, WDT timer duration = don't care
        LPC_WDT->MOD = 1;           // enable WDT for counting/interrupts but not resets
        LPC_WDT->FEED = 0xAA;       // WDT start sequence step 1
        LPC_WDT->FEED = 0x55;       // WDT start sequence step 2

        Wait1mS(100);               // Need to delay before measuring because WDT looses 256 cycles
        start = LPC_WDT->TV;
        Wait1mS(WDOMEASUREDURATION_MS);
        end = LPC_WDT->TV;

        LPC_SYSCON->SYSAHBCLKCTRL &= ~BF_SYSAHBCLKCTRL_WDT;  // Turn off clock to WDT register block
        WDO_clocks = (start-end)*4;
    }

    return WDO_clocks;
}

void InitGPIOForSleep(void)
{
    // Set pins to output and drive low
    // changes need to be made here depending on PCB layout to
    // minimize power consumption
#ifdef DEBUG
    LPC_GPIO0->DIR  = 0xFFE; // Set all GPIO pins as outputs except reset
#else
    LPC_GPIO0->DIR  = 0xFFF; // Set all GPIO pins as outputs
#endif
    LPC_GPIO1->DIR  = 0xFFF; // Set all GPIO pins as outputs
    LPC_GPIO2->DIR  = 0xFFF; // Set all GPIO pins as outputs
    LPC_GPIO3->DIR  = 0xFFF; // Set all GPIO pins as outputs

#ifdef LPCXPRESSO_LPC1343_BOARD
    LPC_GPIO0->DATA = 1;     // Set all GPIO pins low except JTAG reset
#else
    LPC_GPIO0->DATA = 0;     // Set all GPIO pins low
#endif
    LPC_GPIO1->DATA = 0;     // Set all GPIO pins low
#ifdef KEIL_MCB1000_BOARD
    LPC_GPIO2->DATA = 0xFF;  // All pins low except LEDs
#else
    LPC_GPIO2->DATA = 0;     // Set all GPIO pins low
#endif
    LPC_GPIO3->DATA = 0;     // Set all GPIO pins low

    // Configure pins as GPIO without pullup
    LPC_IOCON->PIO2_6 = 0xC0;
    LPC_IOCON->PIO2_0 = 0xC0;
    LPC_IOCON->PIO0_1 = 0xC0;
    LPC_IOCON->PIO1_8 = 0xC0;
    LPC_IOCON->PIO0_2 = 0xC0;
    LPC_IOCON->PIO2_7 = 0xC0;
    LPC_IOCON->PIO2_8 = 0xC0;
    LPC_IOCON->PIO2_1 = 0xC0;
    LPC_IOCON->PIO0_3 = 0xC0;
    LPC_IOCON->PIO0_4 = 0xC0; // I2C pin
    LPC_IOCON->PIO0_5 = 0xC0; // I2C pin
    LPC_IOCON->PIO1_9 = 0xC0;
    LPC_IOCON->PIO3_4 = 0xC0;
    LPC_IOCON->PIO2_4 = 0xC0;
    LPC_IOCON->PIO2_5 = 0xC0;
    LPC_IOCON->PIO3_5 = 0xC0;
    LPC_IOCON->PIO0_6 = 0xC0;
    LPC_IOCON->PIO0_7 = 0xC0;
    LPC_IOCON->PIO2_9 = 0xC0;
    LPC_IOCON->PIO2_10 =0xC0;
    LPC_IOCON->PIO2_2 = 0xC0;
    LPC_IOCON->PIO0_8 = 0xC0;
    LPC_IOCON->PIO0_9 = 0xC0;

    LPC_IOCON->PIO1_10 = 0xC0; // ADC pin
    LPC_IOCON->PIO2_11 = 0xC0;

    LPC_IOCON->PIO3_0 = 0xC0;
    LPC_IOCON->PIO3_1 = 0xC0;
    LPC_IOCON->PIO2_3 = 0xC0;

    LPC_IOCON->PIO1_4 = 0xC0; // ADC pin
    LPC_IOCON->PIO1_11 = 0xC0; // ADC pin
    LPC_IOCON->PIO3_2 = 0xC0;
    LPC_IOCON->PIO1_5 = 0xC0;
    LPC_IOCON->PIO1_6 = 0xC0;
    LPC_IOCON->PIO1_7 = 0xC0;
    LPC_IOCON->PIO3_3 = 0xC0;

    LPC_IOCON->R_PIO0_11 = 0xC1; // ADC pin
    LPC_IOCON->R_PIO1_0  = 0xC1; // ADC pin
    LPC_IOCON->R_PIO1_1  = 0xC1; // ADC pin
    LPC_IOCON->R_PIO1_2 = 0xC1; // ADC pin
#ifndef DEBUG
    LPC_IOCON->RESET_PIO0_0     = 0xC1; // disables reset
    LPC_IOCON->SWCLK_PIO0_10 = 0xC1; // disables SWCLK
    LPC_IOCON->SWDIO_PIO1_3 = 0xC1; // ADC pin, disables SWDIO
#endif
}

int main(void)
{
      LPC_SYSCON->PDRUNCFG      = BF_PDRUNCFG_RUN; // Initialize power to chip modules
    LPC_SYSCON->SYSAHBCLKCTRL = BF_SYSAHBCLKCTRL_RUN; // Initialize clocks

    InitFromPowerAPIDemo();

    InitGPIOForSleep(); // Set all GPIO as outputs driving low

    InitDeepSleep();

    int cycleNum = 1;

    while(1)
    {
    	SetBitsPort0(1<<8);  // Turn on LED on LPCXpresso board
        Wait1mS(5000);
        ClrBitsPort0(1<<8);  // Turn off LED on LPCXpresso board
    	//48 MHz setup begin
		LPC_SYSCON->MAINCLKSEL = 0x01;              //main clock source is the PLL input
		LPC_SYSCON->MAINCLKUEN = 0x00;              //update the main clock source...
		LPC_SYSCON->MAINCLKUEN = 0x01;              //...
		int i = 0;
		for (i = 0; i != 10000; i++);               //wait for a while
		command[0] = 48;                            //system freq 48 MHz
		currentClockSpeed = 48;
		command[1] = PARAM_LOW_CURRENT;             //specify system power for low active current
		(*rom)->pPWRD->set_power(command,result);   //set system power
		if (result[0] != PARAM_CMD_CUCCESS){        //if a failure is reported...
			while(1);                               //... stay in the loop
		}
		command[0] = 12000;                         //PLL's input freq 12000
		command[1] = 48000;                         //CPU's freq 48000
		command[2] = CPU_FREQ_EQU;                  //specify exact frequency
		command[3] = 0;                             //infinitely wait for the PLL to lock
		(*rom)->pPWRD->set_pll(command,result);     //set the PLL
		if ((result[0] != PLL_CMD_CUCCESS)){        //if a failure is reported...
			while(1);                               //... stay in the loop
		}
		fibonacci(30);

			SetBitsPort0(1<<7);  // Turn on LED on LPCXpresso board
			Wait1mS(500);
			ClrBitsPort0(1<<7);  // Turn off LED on LPCXpresso board
		//48 MHz setup end

		//24 MHz setup begin
		LPC_SYSCON->MAINCLKSEL = 0x01;              //main clock source is the PLL input
		LPC_SYSCON->MAINCLKUEN = 0x00;              //update the main clock source...
		LPC_SYSCON->MAINCLKUEN = 0x01;              //...
		for (i = 0; i != 10000; i++);               //wait for a while
		command[0] = 24;                            //system freq 24 MHz
		currentClockSpeed = 24;
		command[1] = PARAM_LOW_CURRENT;             //specify system power for low active current
		(*rom)->pPWRD->set_power(command,result);   //set system power
		if (result[0] != PARAM_CMD_CUCCESS){        //if a failure is reported...
			while(1);                               //... stay in the loop
		}
		command[0] = 12000;                         //PLL's input freq 12000
		command[1] = 24000;                         //CPU's freq 24000
		command[2] = CPU_FREQ_EQU;                  //specify exact frequency
		command[3] = 0;                             //infinitely wait for the PLL to lock
		(*rom)->pPWRD->set_pll(command,result);     //set the PLL
		if ((result[0] != PLL_CMD_CUCCESS)){        //if a failure is reported...
			while(1);                               //... stay in the loop
		}
		fibonacci(30);

			SetBitsPort0(1<<7);  // Turn on LED on LPCXpresso board
			Wait1mS(500);
			ClrBitsPort0(1<<7);  // Turn off LED on LPCXpresso board
		//24 MHz setup end

		//18 MHz setup begin
		LPC_SYSCON->MAINCLKSEL = 0x01;              //main clock source is the PLL input
		LPC_SYSCON->MAINCLKUEN = 0x00;              //update the main clock source...
		LPC_SYSCON->MAINCLKUEN = 0x01;              //...
		for (i = 0; i != 10000; i++);               //wait for a while
		command[0] = 18;                            //system freq 18 MHz
		currentClockSpeed = 18;
		command[1] = PARAM_LOW_CURRENT;             //specify system power for low active current
		(*rom)->pPWRD->set_power(command,result);   //set system power
		if (result[0] != PARAM_CMD_CUCCESS){        //if a failure is reported...
			while(1);                               //... stay in the loop
		}
		command[0] = 12000;                         //PLL's input freq 12000
		command[1] = 18000;                         //CPU's freq 18000
		command[2] = CPU_FREQ_EQU;                  //specify exact frequency
		command[3] = 0;                             //infinitely wait for the PLL to lock
		(*rom)->pPWRD->set_pll(command,result);     //set the PLL
		if ((result[0] != PLL_CMD_CUCCESS)){        //if a failure is reported...
			while(1);                               //... stay in the loop
		}
		fibonacci(30);

			SetBitsPort0(1<<7);  // Turn on LED on LPCXpresso board
			Wait1mS(500);
			ClrBitsPort0(1<<7);  // Turn off LED on LPCXpresso board
		//18 MHz setup end

		//3 MHz setup begin
		LPC_SYSCON->MAINCLKSEL = 0x01;              //main clock source is the PLL input
		LPC_SYSCON->MAINCLKUEN = 0x00;              //update the main clock source...
		LPC_SYSCON->MAINCLKUEN = 0x01;              //...
		for (i = 0; i != 10000; i++);               //wait for a while
		command[0] = 3;                             //system freq 3 MHz
		currentClockSpeed = 3;
		command[1] = PARAM_LOW_CURRENT;             //specify system power for low active current
		(*rom)->pPWRD->set_power(command,result);   //set system power
		if (result[0] != PARAM_CMD_CUCCESS){        //if a failure is reported...
			while(1);                               //... stay in the loop
		}
		command[0] = 12000;                         //PLL's input freq 12000
		command[1] = 3000;                          //CPU's freq 3000
		command[2] = CPU_FREQ_EQU;                  //specify exact frequency
		command[3] = 0;                             //infinitely wait for the PLL to lock
		(*rom)->pPWRD->set_pll(command,result);     //set the PLL
		if ((result[0] != PLL_CMD_CUCCESS)){        //if a failure is reported...
			while(1);                               //... stay in the loop
		}
		fibonacci(30);
		//3 MHz setup end

		SetBitsPort0(1<<7);  // Turn on LED on LPCXpresso board
		Wait1mS(500);
		ClrBitsPort0(1<<7);  // Turn off LED on LPCXpresso board

        EnterDeepSleep();

        int ii;
        for(ii = 0; ii < cycleNum; ii++)
        {
    		SetBitsPort0(1<<7);  // Turn on LED on LPCXpresso board
    		Wait1mS(2000);
    		ClrBitsPort0(1<<7);  // Turn off LED on LPCXpresso board
    		Wait1mS(2000);
        }
        cycleNum++;
    }

    while(1);
}
