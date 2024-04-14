#include <stdlib.h>

uint8_t* xbee_api_buffer = NULL;
uint16_t xbee_api_buffer_size = 0;

// Note that this function modifies xbee_api_buffer
// by appending the checksum to it
// This also assumes that xbee_api_buffer is well-formed
// (its length bytes are correct, and has appropriate data
// up to and not including the checksum byte) and has
// enough capacity to add the checksum byte
void xbee_api_add_checksum() {
	// Third (LSB) and second (MSB) bytes describe the packet's length
	uint16_t len = (xbee_api_buffer[1] << 8) | xbee_api_buffer[2];

	// Step 1: Compute sum of packet bytes
	// Note that this length does not include the first 0x7E byte and
	// the length bytes, so offset by 3 when summing
	uint32_t sum = 0;
	for(int i = 3; i < (len + 3); i++) {
		sum += (uint8_t)(xbee_api_buffer[i]);
	}

	// Step 2 and 3: Get the last byte of the sum, and
	// subtract it from 0xFF
	uint8_t checksum = 0xFF - (uint8_t)(0xFF & sum);

	//Finally, append checksum to packet
	xbee_api_buffer[len + 4 - 1] = checksum;
}

// Need to specify len, because it is possible that
// the length bytes have been corrupted
// Note that len does not include the checksum byte, so
// this needs to be accounted for later
bool xbee_api_verify_checksum() {
	// Third (LSB) and second (MSB) bytes describe the packet's length
	uint16_t len = (xbee_api_buffer[1] << 8) | xbee_api_buffer[2];

	// Step 1: Compute sum of packet bytes, including checksum
	// Note that this length does not include the first 0x7E byte and
	// the length bytes, so offset by 3 when summing
	uint32_t sum = 0;
	for(int i = 3; i < (len + 3 + 1); i++) {
		sum += (uint8_t)(xbee_api_buffer[i]);
	}

	// Step 2: Check if LSB equals 0xFF
	return ((0xFF & sum) == 0xFF);
}

// Note that the len specified is the length of the
// packet without the Start Delimeter (byte 0), Length
// (bytes 1-2), and Checksum (the last byte)
void xbee_api_ensure_buffer_capacity(uint16_t len) {
	if(xbee_api_buffer_size < (len + 4)) {
		xbee_api_buffer = realloc(xbee_api_buffer, len + 4);
	}
}

void xbee_api_preconfigure_buffer(uint8_t type, uint16_t len) {
	xbee_api_ensure_buffer_capacity(len);

	xbee_api_buffer[0] = 0x7E; // The start delimiter
	xbee_api_buffer[1] = 0xFF & (len >> 8); // The length MSB
	xbee_api_buffer[2] = 0xFF & (len >> 0); // The length LSB
	xbee_api_buffer[3] = type;
}

bool xbee_api_read_api_packet() {
	int count = 0;
	uint16_t len = 0;
	while(true) {
		uint8_t data = xbee_api_uart_getchar();

		switch(count) {
			case 0: {
				if(data != 0x7E) return false;
				break;
			}
			case 1: {
				len = data;
				break;
			}
			case 2: {
				len = (len << 8) | data;
				break;
			}
		}

		xbee_api_buffer[count++] = data;

		if(count == (len + 4)) break;
	}

	return xbee_api_verify_checksum();
}

void xbee_api_send_AT(char* at_command, char* value, uint8_t value_len) {
	// One byte for the Frame Type
	// One byte for the Frame ID
	// Two bytes for the AT command
	uint16_t len = 1 + 1 + 2 + value_len;

	xbee_api_preconfigure_buffer(0x08, len);
	xbee_api_buffer[4] = 0x01; // Frame ID, must be non-zero to get data back
	xbee_api_buffer[5] = at_command[0]; // Copy over the AT command
	xbee_api_buffer[6] = at_command[1];

	// Copy over the parameter
	for(int i = 0; i < value_len; i++) {
		xbee_api_buffer[7 + i] = value[i];
	}

	// Append the checksum
	xbee_api_add_checksum();

	// Need to account for the 4 extra bytes of start
	// delimiter, length, and checksum
	xbee_api_uart_write(xbee_api_buffer, len + 4);
}

bool xbee_api_receive_AT() {
	if(!xbee_api_read_api_packet()) {
		return false;
	}

	if(!xbee_api_buffer[4]) {
		// Non-zero status value means error
		return false;
	}

	return true;
}

// Get the name by querying the NI variable with an AT command
void xbee_api_get_name() {
	// First, issue the command with no parameters to read value
	xbee_api_send_AT("NI", "", 0);
	if(!xbee_api_receive_AT()) return;

	uint16_t result_len = (xbee_api_buffer[1] << 8) | xbee_api_buffer[2];

	for(int i = 8; i < result_len + 4 - 1; i++) {
		printf("%c", xbee_api_buffer[i]);
	}

	printf("\n");
}

void xbee_api_transmit_data(char* data, uint16_t data_len, uint64_t address) {
	// One byte for the Frame Type
	// One byte for the Frame ID
	// Eight bytes for 64-bit Destionation Address
	// Two bytes for 16-bit Destionation Address
	// One byte for Boradcast Radius
	// One byte for Transmit Options
	// data_len bytes for payload
	uint16_t len = 1 + 1 + 8 + 2 + 1 + 1 + data_len;

	xbee_api_preconfigure_buffer(0x10, len);
	xbee_api_buffer[4] = 0x01; // Frame ID, must be non-zero to get data back
	// Copy over 64-bit address,
	// use bit shifts to get correct endianness
	for(int i = 0; i < 8; i++) {
		xbee_api_buffer[i + 5] = (0xFF & (address >> (7 - i)));
	}
	// To use 64-bit address, 16-bit addrexx must be 0xFFFE
	xbee_api_buffer[13] = 0xFF;
	xbee_api_buffer[14] = 0xFE;
	xbee_api_buffer[15] = 0; // Not a broadcast transmission, so ignored
	xbee_api_buffer[16] = 0; // Use default value specified in AT+TO
	//Copy over the data
	for(int i = 0; i < data_len; i++) {
		xbee_api_buffer[i + 17] = data[i];
	}

	// Append the checksum
	xbee_api_add_checksum();

	// Need to account for the 4 extra bytes of start
	// delimiter, length, and checksum
	xbee_api_uart_write(xbee_api_buffer, len + 4);

	bool success;

	success = xbee_api_read_api_packet();

	if(!success) {
		printf("Could not read response packet\n");
	}

	if(xbee_api_buffer[8] != 0) {
		printf("Transmit failed: Reason 0x%02X\n", xbee_api_buffer[8]);
	}
}

void xbee_api_receive_data() {

}
