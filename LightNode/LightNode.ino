void xbee_api_uart_write(uint8_t* data, uint16_t len) {
  for(int i = 0; i < len; i++) {
    Serial.write(data[i]);
  }
}

uint8_t xbee_api_uart_getchar() {
  int value;
  while((value = Serial.read()) == -1);

  return (uint8_t)value;
}

#include "xbee_api.h"


void setup() {
  Serial.begin(9600, SERIAL_8N1);
}

void loop() {
    uint16_t number = 2024;
    char data[2];
    data[0] = 0xFF & (number >> 8);
    data[1] = 0xFF & (number >> 0);
    xbee_api_transmit_data(data, 2, 0x00000000);
    delay(1000);

}
