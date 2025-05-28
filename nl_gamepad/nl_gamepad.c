/*
 * nl_gamepad.c - demo of using the CH32V003 Nav LCD board as a 5-way HID gamepad
 * 05-27-25 E. Brombaugh - mostly ganked from rv003usb demo-gamepad example
 */

#include "ch32fun.h"
#include <stdio.h>
#include <string.h>
#include "rv003usb.h"
#include "systick.h"

int main()
{
	SystemInit();
	systick_init();
	usb_setup();
	while(1);
}

void usb_handle_user_in_request( struct usb_endpoint * e, uint8_t * scratchpad, int endp, uint32_t sendtok, struct rv003usb_internal * ist )
{
	if( endp )
	{
		static uint8_t tsajoystick[1] = { 0x00 };
		tsajoystick[0]  = (SysTick_get_button_st(0)<<0);
		tsajoystick[0] |= (SysTick_get_button_st(1)<<1);
		tsajoystick[0] |= (SysTick_get_button_st(2)<<2);
		tsajoystick[0] |= (SysTick_get_button_st(3)<<3);
		tsajoystick[0] |= (SysTick_get_button_st(4)<<4);
		
		usb_send_data( tsajoystick, 1, 0, sendtok );
	}
	else
	{
		// If it's a control transfer, nak it.
		usb_send_empty( sendtok );
	}
}


