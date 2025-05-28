/*
 * systick.h - single-file header for Systick IRQ
 * 07-21-23 E. Brombaugh
 */

#ifndef __systick__
#define __systick__

#include "debounce.h"

/*
 * Pin assignments for 20-pin package
 * Pin # | GPIO     | Signal   | Function
 *  3      PD6    	  JS_NW      Joystick NW switch
 *  5      PA1        JS_NE		 Joystick NE switch
 *  6      PA2   	  JS_CTR	 Joystick Center switch
 *  8      PD0        JS_SE      Joystick SE switch
 *  10     PC0        JS_SW      Joystick SW switch
 */

/* some bit definitions for systick regs */
#define SYSTICK_SR_CNTIF (1<<0)
#define SYSTICK_CTLR_STE (1<<0)
#define SYSTICK_CTLR_STIE (1<<1)
#define SYSTICK_CTLR_STCLK (1<<2)
#define SYSTICK_CTLR_STRE (1<<3)
#define SYSTICK_CTLR_SWIE (1<<31)

#define NUM_BTNS 5

volatile uint32_t systick_cnt;
debounce_state dbs[NUM_BTNS];

const uint8_t btn_pin[NUM_BTNS] = 
{
	6,		// PD6 = NW / UP
	1,		// PA1 = NE / RIGHT
	2,		// PA2 = CTR
	0,		// PD0 = SE / DN
	0		// PC0 = SW / LEFT
};

GPIO_TypeDef *btn_port[NUM_BTNS] =
{
	GPIOD,		// PD6 = NW
	GPIOA,		// PA1 = NE
	GPIOA,		// PA2 = CTR
	GPIOD,		// PD0 = SE
	GPIOC		// PC0 = SW
};

const char *btn_name[NUM_BTNS] =
{
	"UP",
	"RIGHT",
	"CENTER",
	"DOWN",
	"LEFT"
};

/*
 * Start up the SysTick IRQ and init button debounce
 */
void systick_init(void)
{
	uint32_t temp;

	/* turn on ports */
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;

	for(uint8_t i=0;i<NUM_BTNS;i++)
	{
		/* setup button inputs with pullup */
		temp = btn_port[i]->CFGLR & ~(0xf<<(4*btn_pin[i]));
		temp |= (GPIO_CNF_IN_PUPD)<<(4*btn_pin[i]);
		btn_port[i]->CFGLR = temp;
		btn_port[i]->BSHR = (1<<btn_pin[i]);	// pull up
	
		/* init the debouncer */
		init_debounce(&dbs[i], 10);
	}

	/* disable default SysTick behavior */
	SysTick->CTLR = 0;
	
	/* enable the SysTick IRQ */
	NVIC_EnableIRQ(SysTicK_IRQn);
	
	/* Set the tick interval to 1ms for normal op */
	SysTick->CMP = (FUNCONF_SYSTEM_CORE_CLOCK/1000)-1;
	
	/* Start at zero */
	SysTick->CNT = 0;
	systick_cnt = 0;
	
	/* Enable SysTick counter, IRQ, HCLK/1 */
	SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE |
					SYSTICK_CTLR_STCLK;
}

/*
 * restart the SysTick IRQ after missed interrupt
 */
void SysTick_Restart(void)
{
	/* push the IRQ out ahead of now */
	SysTick->CMP = SysTick->CNT + (FUNCONF_SYSTEM_CORE_CLOCK/1000)-1;
	
	/* clear IRQ */
	SysTick->SR = 0;
}

/*
 * SysTick ISR counts ticks & updates button state
 * note - the __attribute__((interrupt)) syntax is crucial!
 */
void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
	// move the compare further ahead in time.
	// as a warning, if more than this length of time
	// passes before triggering, you may miss your
	// interrupt.
	SysTick->CMP += (FUNCONF_SYSTEM_CORE_CLOCK/1000);

	/* clear IRQ */
	SysTick->SR = 0;

	/* update counter */
	systick_cnt++;
	
	/* update button debounce */
	for(int i=0;i<NUM_BTNS;i++)
		debounce(&dbs[i], (~btn_port[i]->INDR >> btn_pin[i])&1);
}

/*
 * get button state
 */
uint8_t SysTick_get_button_st(uint8_t btn)
{
	/* detect illegal button */
	if(btn >= NUM_BTNS)
		return 0;
	
	/* check for rising edge and reset */
	uint8_t result = dbs[btn].state;
	return result;
}

/*
 * get button edge
 */
uint8_t SysTick_get_button_re(uint8_t btn)
{
	/* detect illegal button */
	if(btn >= NUM_BTNS)
		return 0;
	
	/* check for rising edge and reset */
	uint8_t result = dbs[btn].re;
	if(result)
		dbs[btn].re = 0;
	return result;
}

/*
 * compute goal for Systick counter based on desired delay in ticks
 */
uint32_t SysTick_goal(uint32_t ticks)
{
	return ticks + systick_cnt;
}

/*
 * return FALSE if goal is reached
 */
uint32_t SysTick_check(uint32_t goal)
{
    /**************************************************/
    /* DANGER WILL ROBINSON!                          */
    /* the following syntax is CRUCIAL to ensuring    */
    /* that this test doesn't have a wrap bug         */
    /**************************************************/
	return (((int32_t)systick_cnt - (int32_t)goal) < 0);
}

#endif