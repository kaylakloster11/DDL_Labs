/*****************************************************************************
 *   adctest.c:  main C entry file for NXP LPC11xx Family Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.12.07  ver 1.00    Preliminary version, first Release
 *
******************************************************************************/
#include "driver_config.h"
#include "target_config.h"

#include "adc.h"
#include "gpio.h"
#include "debug_printf.h"

#ifdef ADC_DEBUG
uint8_t ConvertDigital ( uint8_t digital )
{
  uint8_t ascii_char;

  if ( (digital >= 0) && (digital <= 9) )
  {
	ascii_char = digital + 0x30;	/* 0~9 */
  }
  else
  {
	ascii_char = digital - 0x0A;
	ascii_char += 0x41;				/* A~F */
  }
  return ( ascii_char );
}
#endif

#ifdef ADC_DEBUG
#define BAR_OUTPUT_LENGTH 60
#define ADC_COUNT_MAX 1023
#define CHANNEL_WIDTH 2
#define VOLT_WIDTH 4
#define MAX_BAR_LENGTH (BAR_OUTPUT_LENGTH-CHANNEL_WIDTH-1-VOLT_WIDTH-1-2)
#define SUPPLY_VOLTAGE 3.30

void FillString(char *s, char c, uint32_t len)
{
	if(!len)
		goto empty;
	while(--len)
		*s++ = c;
empty:
	*s = 0;
}
void ADCBar(int32_t Channel, uint32_t Counts)
{
	uint32_t ValueBarLen = ((Counts * (MAX_BAR_LENGTH+1)) - (ADC_COUNT_MAX/2 - 1)) / ADC_COUNT_MAX;
	char ValueBarBuf[BAR_OUTPUT_LENGTH+1];
	double ValueVolts = Counts*SUPPLY_VOLTAGE/ADC_COUNT_MAX;

	if(Channel >= 0)
	{
		if(ValueBarLen > MAX_BAR_LENGTH) ValueBarLen = MAX_BAR_LENGTH;
		FillString(ValueBarBuf, '#', ValueBarLen);
		debug_printf("%2d %d.%02dV |%s%*s|\n", (int)Channel,
				(int)ValueVolts, (int)((ValueVolts-(int)ValueVolts)*100),
				ValueBarBuf, (int)(MAX_BAR_LENGTH-ValueBarLen), "");
	}
	else
	{
		FillString(ValueBarBuf, '-', MAX_BAR_LENGTH+2);
		debug_printf("%-9s%s\n", "", ValueBarBuf);
	}
}
#endif

/******************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
	 /* Basic chip initialization is taken care of in SystemInit() called
	   * from the startup code. SystemInit() and chip settings are defined
	   * in the CMSIS system_<part family>.c file.
	   */
  uint32_t i;
  uint32_t ADC_VAL = 0;
  double adc_Volts = 0;
  double V_thresh = (SUPPLY_VOLTAGE/2);
  //uint16_t freq_samp = 0;

  /* Set LED port pin to output */
    GPIOSetDir(LED_PORT_R, LED_BIT_R, 1 );
  /* Initialize ADC  */
  ADCInit( ADC_CLK );

  /* LED Initialization code here */


  while(1)
  {
	/* Read one sample from the ADC port 'AD0' */
	ADC_VAL = ADCRead( 0 );
	//while ( !ADCIntDone );
	//ADCIntDone = 0;
	adc_Volts = ADC_VAL*SUPPLY_VOLTAGE/ADC_COUNT_MAX;

//#ifdef ADC_DEBUG
	/* Print ADC Voltage as a real-time bar graph */
	//ADCBar(0, ADCValue[0]);
//#endif

	//light LED when voltage is greater than Vcc/2
	if (adc_Volts > V_thresh){
		GPIOSetValue( LED_PORT_R, LED_BIT_R, LED_ON );
	}
	else{
		GPIOSetValue( LED_PORT_R, LED_BIT_R, LED_OFF );
	}

/*	if(adc_Volts >= VCC_max){
		freq_sam++;
	}
	if ( (timer32_0_counter%(LED_TOGGLE_TICKS/COUNT_MAX)) < ((LED_TOGGLE_TICKS/COUNT_MAX)/2) )
	{
	  period = freq_sam/( CLOCK FREQUENCY TIME );
	  freq_CALC = 1/period;
	  freq_sam = 0;
	} */

  }
}

/******************************************************************************
**                            End Of File
******************************************************************************/

