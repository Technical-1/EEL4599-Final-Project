#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/uart.h"
#include "hardware/adc.h"

#define UART uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

#define UART_TX_PIN 0
#define UART_RX_PIN 1

void xbee_api_uart_write(uint8_t* data, uint16_t len) {
	uart_write_blocking(UART, data, len);
}

uint8_t xbee_api_uart_getchar() {
	return (uint8_t)uart_getc(UART);
}

#include "xbee_api.h"

int main() {
	stdio_init_all();

	uart_init(UART, BAUD_RATE);

	gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	// Disable flow control
	uart_set_hw_flow(UART, false, false);

	// Set the uart format
	uart_set_format(UART, DATA_BITS, STOP_BITS, PARITY);

	// Enable the FIFO
	uart_set_fifo_enabled(UART, true);

	// Enable the ADC
	adc_init();
	adc_gpio_init(25);
	adc_select_input(0);

	while(true) {
		//xbee_api_get_name();

		uint16_t number = adc_read();

		char data[2];
		data[0] = 0xFF & (number >> 8);
		data[1] = 0xFF & (number >> 0);
		xbee_api_transmit_data(data, 2, 0x00000000);

		sleep_ms(1000);
		//tight_loop_contents();
	}
}
