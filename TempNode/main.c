#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/uart.h"
#include "hardware/adc.h"
#include <math.h>

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

// Compute the resistance using the voltage divider equation,
// recall that ADC is 12-bit and using 10k resistor in divider
float compute_resistance(float adc_value) {
	return 7.3e3 * (( (1 << 12) / adc_value ) - 1);
}

// Compute the temperature, in Fahrenheit
float compute_temperature(float R) {
	static const float B = 3435;
	static const float T0 = 25 + 273.15; // In Kelvin
	static const float R0 = 10e3;

	const float temp_K = 1 / (1 / T0 + 1/B * log(R / R0));
	const float temp_C = temp_K - 273.15;
	return (9 * temp_C) / 5 + 32;
}

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
	volatile uint64_t start = time_us_64();
	volatile uint64_t end, diff;

	gpio_set_pulls(25, false, false);

	while(true) {
		//xbee_api_get_name();

		uint64_t total = 0;
		uint32_t count = 0;

		// Average all samples over the one-second interval
		do {
			total += adc_read();
			count++;
			end = time_us_64();
			diff = end - start;
		} while(diff < (uint64_t)40000000);
		start = end; //Update the start time for the next iteration

		float average = ((float) total) / count;

		float temp_F = compute_temperature(compute_resistance(average));

		printf("The temperature is %.02fF\n", temp_F);
		int16_t number = (int16_t)(temp_F * 10);

		char data[2];
		data[0] = 0xFF & (number >> 8);
		data[1] = 0xFF & (number >> 0);
		xbee_api_transmit_data(data, 2, 0x00000000);

		//tight_loop_contents();
	}
}
