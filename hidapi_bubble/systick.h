/*
 * systick.h - single-file header for Systick IRQ
 * handles 8-digit 7-segment LED refresh
 * 07-21-23 E. Brombaugh
 */

#ifndef __systick__
#define __systick__

#include <string.h>

/* some bit definitions for systick regs */
#define SYSTICK_SR_CNTIF (1<<0)
#define SYSTICK_CTLR_STE (1<<0)
#define SYSTICK_CTLR_STIE (1<<1)
#define SYSTICK_CTLR_STCLK (1<<2)
#define SYSTICK_CTLR_STRE (1<<3)
#define SYSTICK_CTLR_SWIE (1<<31)

#define NUM_DIGITS 8
uint8_t brtdig, cathode, anode[NUM_DIGITS];

#define SEGMASK_C 0xF9
#define SEGMASK_D 0x05

/* 
 * initialize segment outputs
 */
void init_seg(void)
{
	uint32_t temp;

	// GPIO C & D
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
	
	// C[0,3,4-7]
	temp = GPIOC->CFGLR & ~((0xf<<(4*0)) | (0xf<<(4*3)) | (0xf<<(4*4)) | (0xf<<(4*5)) | (0xf<<(4*6)) | (0xf<<(4*7)));
	temp |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*3) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*5) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*6) |
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*7);
	GPIOC->CFGLR = temp;
	temp = GPIOC->OUTDR & ~SEGMASK_C;
	GPIOC->OUTDR = temp;
	
	// D[0,2]
	temp = GPIOD->CFGLR & ~((0xf<<(4*0)) | (0xf<<(4*2)));
	temp |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*0) | 
		(GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*2);
	GPIOD->CFGLR = temp;
	temp = GPIOC->OUTDR & ~SEGMASK_C;
	GPIOC->OUTDR = temp;
}

/*
 * 7 seg mapping
 * PGFEDCBA 
 */

/* table of 7-seg digit mapping */
const uint8_t segments[16] = {
	0x3f,	// 0
	0x06,	// 1
	0x5b,	// 2
	0x4f,	// 3
	0x66,	// 4
	0x6d,	// 5
	0x7d,	// 6
	0x07,	// 7
	0x7f,	// 8
	0x6f,	// 9
	0x77,	// A
	0x7c,	// b
	0x58,	// c
	0x5e,	// d
	0x79,	// E
	0x71,	// F
};

/*
 * map segments to bits in ports C & D
 * hex digit in low nybble bits 3-0
 * DP state in bit 4
 */
void drive_seg(uint8_t segs)
{
	uint32_t temp, bits_out;
	
	/* GPIOC bits PAEDGXXF */
	bits_out  = (segs & 0x80);		// Decimal point
	bits_out |= (segs & 0x01) << 6;		// A
	bits_out |= (segs & 0x08) << 1;		// D
	bits_out |= (segs & 0x10) << 1;		// E
	bits_out |= (segs & 0x20) >> 5;		// F
	bits_out |= (segs & 0x40) >> 3;		// G
	temp = GPIOC->OUTDR & ~SEGMASK_C;
	GPIOC->OUTDR = temp | bits_out;
	
	/* GPIOD bits XXXXXCXB */
	bits_out  = (segs & 0x02) >> 1;
	bits_out |= (segs & 0x04);
	temp = GPIOD->OUTDR & ~SEGMASK_D;
	GPIOD->OUTDR = temp | bits_out;
}

#define SER_DAT_PORT GPIOD
#define SER_DAT_PIN 6
#define SER_LAT_PORT GPIOA
#define SER_LAT_PIN 1
#define SER_CLK_PORT GPIOA
#define SER_CLK_PIN 2
#define SET_SER_DAT() (SER_DAT_PORT->BSHR|=(1<<(0+SER_DAT_PIN)))
#define SET_SER_LAT() (SER_LAT_PORT->BSHR|=(1<<(0+SER_LAT_PIN)))
#define SET_SER_CLK() (SER_CLK_PORT->BSHR|=(1<<(0+SER_CLK_PIN)))
#define CLR_SER_DAT() (SER_DAT_PORT->BSHR|=(1<<(16+SER_DAT_PIN)))
#define CLR_SER_LAT() (SER_LAT_PORT->BSHR|=(1<<(16+SER_LAT_PIN)))
#define CLR_SER_CLK() (SER_CLK_PORT->BSHR|=(1<<(16+SER_CLK_PIN)))

/*
 * initialize 74HC595 shift reg for digit drive
 */
void init_dig(void)
{
	uint32_t temp;

	/* map PA1, PA2 for GPIO instead of HSE */
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;
	AFIO->PCFR1 &= ~GPIO_Remap_PA1_2;

	/* setup HC595 bitbang outputs on PD6, PA1, PA2 */
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD;
	
	temp = SER_DAT_PORT->CFGLR & ~(0xf<<(4*SER_DAT_PIN));
	temp |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*SER_DAT_PIN);
	SER_DAT_PORT->CFGLR = temp;
	
	temp = SER_LAT_PORT->CFGLR & ~(0xf<<(4*SER_LAT_PIN));
	temp |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*SER_LAT_PIN);
	SER_LAT_PORT->CFGLR = temp;
	
	temp = SER_CLK_PORT->CFGLR & ~(0xf<<(4*SER_CLK_PIN));
	temp |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*SER_CLK_PIN);
	SER_CLK_PORT->CFGLR = temp;
	
	/* all bits low */
	CLR_SER_DAT();
	CLR_SER_LAT();
	CLR_SER_CLK();
}

/*
 * send bits to 74HC595
 */
void send_dig(uint8_t digits)
{
	uint8_t cnt;
	
	for(cnt=0;cnt<NUM_DIGITS;cnt++)
	{
		/* send data */
		if(digits & 0x80)
			SET_SER_DAT();
		else
			CLR_SER_DAT();
		
		/* clock it */
		SET_SER_CLK();
		CLR_SER_CLK();
		
		/* advance bits */
		digits <<= 1;
	}
	
	/* latch it */
	SET_SER_LAT();
	CLR_SER_LAT();
}

/*
 * Start up the SysTick IRQ and init button debounce
 */
void systick_init(void)
{
	/* init segment driver */
	init_seg();
	
	/* init digit driver */
	init_dig();
	
	/* disable default SysTick behavior */
	SysTick->CTLR = 0;
	
	/* enable the SysTick IRQ */
	NVIC_EnableIRQ(SysTicK_IRQn);
	
	/* Set the tick interval to 1ms for normal op */
	SysTick->CMP = (FUNCONF_SYSTEM_CORE_CLOCK/1000)-1;
	
	/* Start at zero */
	SysTick->CNT = 0;
	
	/* init state */
	cathode = 0;
	brtdig = NUM_DIGITS;
	memset(anode, 0, NUM_DIGITS);
	
	/* Enable SysTick counter, IRQ, HCLK/1 */
	SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE |
					SYSTICK_CTLR_STCLK;
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
	
	/* disable all digits */
	send_dig(0);
	
	if(cathode < NUM_DIGITS)
	{
		/* normal digit - fetch next digit data */
		drive_seg(anode[cathode]);
	
		/* turn on selected cathode */
		send_dig(1<<cathode);
	}
#if 0
	else
	{
		if(brtdig < NUM_DIGITS)
		{
			/* send bright digit data */
			drive_seg(anode[brtdig]);
	
			/* turn on brtdig cathode */
			send_dig(1<<brtdig);
		}
	}
#endif
	
	/* update cathode state */
	cathode++;
	if(cathode > NUM_DIGITS+3)
		cathode = 0;
}

/*
 * set digit data as hex
 */
void set_digit_hex(uint8_t digit, uint8_t val)
{
	if(digit >= NUM_DIGITS)
		return;
	
	/* get hex to segments  and decimal point */
	anode[digit] = segments[val&0xf] | (val&0x10)<<3;
}

/*
 * set digit data as raw byte
 */
void set_digit_raw(uint8_t digit, uint8_t data)
{
	if(digit >= NUM_DIGITS)
		return;
	
	anode[digit] = data;
}

/* set bright digit 0-7, others disable */
void set_digit_brt(uint8_t digit)
{
	brtdig = digit;
}
#endif