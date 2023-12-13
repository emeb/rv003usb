#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// We borrow the combined hidapi.c from minichlink.
//
// This is for total perf testing.

#include "hidapi.c"

int main()
{
	uint32_t cnt = 0;
	
	hid_device * hd = hid_open( 0x1209, 0xd003, L"CUSTOMDEVICE000"); // third parameter is "serial"
	if( !hd )
	{
		fprintf( stderr, "Error: Failed to open device.\n" );
		return -4;
	}
	else
		printf("Opened HIDAPI device\n");

	// Size of buffers must match the report descriptor size in the special_hid_desc
	//  NOTE: You are permitted to have multiple entries.
	uint8_t buffer0[255] = { 0 }; // NOTE: This must be ONE MORE THAN what is in the hid descriptor.
	uint8_t buffer1[255] = { 0 };
	int r;
	int i;
	int j;
	int retries = 0;
	for( j = 0 ; j < 4096 ; j++)
	{
		buffer0[0] = 0xaa; // First byte must match the ID.

#if 0
		// But we can fill in random for the rest.
		for( i = 1; i < sizeof( buffer0 ); i ++ )
			buffer0[i] = rand(); 
#else
		// 32-bit counter
		for(i = 0; i < 4; i++)
			buffer0[i+1] = (cnt>>(8*i)) & 0xff;
		cnt++;
#endif
		
		if( buffer0[1] == 0xa4 ) buffer0[1]++;

		retrysend:
		r = hid_send_feature_report( hd, buffer0, sizeof(buffer0) );
		if( r != sizeof(buffer0) )
		{
			fprintf( stderr, "Warning: HID Send fault (%d) Retrying\n", r );
			retries++;
			if( retries > 10 ) break;
			goto retrysend;
		}

		retries = 0;
		printf( "<" ); // Print this out meaning we sent the data.

		memset( buffer1, 0xff, sizeof( buffer1 ) );
		buffer1[0] = 0xaa; // First byte must be ID.

		r = hid_get_feature_report( hd, buffer1, sizeof(buffer1) );

		printf( ">" );

		if( r != sizeof( buffer1 ) && r != sizeof( buffer1 ) + 1) { printf( "Got %d\n", r ); break; }

		// Validate the scratches matched.
		if( memcmp( buffer0, buffer1, sizeof( buffer0 ) ) != 0 ) 
		{
			printf( "%d: ", r );
			for( i = 0; i < r; i++ )
				printf( "[%d] %02x>%02x %s", i, buffer0[i], buffer1[i], (buffer1[i] != buffer0[i])?"MISMATCH ":""  );
			printf( "\n" );
			break;
		}
		else
			printf("tx & rx matched\n");
		
	}

	hid_close( hd );
}

