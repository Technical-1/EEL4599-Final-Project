#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

// sudo stty -F /dev/ttyACM1 9600 cs8 -cstopb -parenb raw

uint16_t len = 0;
int idx = 0;

uint8_t payload[100];
int payload_count = 0;

void handle_data(uint8_t data) {
	if(idx == 0) {
		if(data != 0x7E)  return;
	} else if(idx == 1) {
		len = data;
	} else if(idx == 2) {
		len = (len << 8) | data;
	} else if(idx >= 17 && idx != ((len + 4 )- 1)) {
		// Skip other information, just want payload
		payload[idx - 17] = data;
		payload_count++;
	}

	if(idx == ((len + 4) - 1)) {
		// Output the payload
		uint16_t temp = (payload[0] << 8) | (payload[1]);

		printf("The temperature is: %.01f\n", ((float)temp) / 10);

		idx = 0;
		payload_count = 0;
		return;
	}

	idx++;
}

int main() {
	int fd = open("/dev/ttyACM1",O_RDWR | O_NOCTTY);// | O_NDELAY);

	if(fd == -1) {
		printf("Could not open serial device\n");
		return -1;
	}

	while(1) {
		uint8_t data[40];

		int len = read(fd, &data, 40);

		for(int i = 0; i < len; i++) {
			handle_data(data[i]);
		}

	}

	close(fd);
}
