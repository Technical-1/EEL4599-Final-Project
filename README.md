# EEL4599 Final Project — Wireless Sensor Network

A multi-node wireless sensor network built around XBee/802.15.4 radios. Battery-powered sensor nodes measure temperature and light, transmit readings over the air as binary XBee API frames, and a WiFi-connected coordinator forwards those readings to the cloud (ThingSpeak) for logging and display.

This was a two-person final project for EEL4599. My focus was the **base-station coordinator** — the WiFi/cloud side that receives radio data and pushes it to ThingSpeak — alongside the shared XBee API frame work.

## System at a glance

```
[Temp Node]  --\
                 >--(XBee 802.15.4 RF)-->  [Coordinator] --(WiFi/HTTP)--> [ThingSpeak cloud]
[Light Node] --/                            + OLED status
```

## Features

- **Portable XBee API protocol layer** — a single C header (`xbee_api.h`) builds and parses XBee API frames (AT commands, transmit requests, receive packets) with checksum generation/verification, reused unchanged across Raspberry Pi Pico and Arduino targets.
- **Temperature node** — Raspberry Pi Pico reads a thermistor via the 12-bit ADC, converts resistance to temperature with the B-parameter (Steinhart–Hart) equation, and oversamples across a fixed interval to reduce noise before transmitting.
- **Light node** — Arduino-based node that reads a luminosity sensor and transmits lux readings over the same XBee frame format.
- **WiFi coordinator** — receives XBee data over UART and uploads it to ThingSpeak over HTTP; an SSD1306 OLED shows connection status and the assigned IP.
- **Host-side tools** — small Python and C utilities for sniffing and decoding XBee API frames during bring-up and debugging.

## Tech Stack

- **Languages**: C (embedded), CircuitPython / MicroPython, Arduino C++, Python (host tools)
- **Microcontrollers**: Raspberry Pi Pico (RP2040), Arduino, ESP8266, Adafruit Feather + ESP32 (AirLift) co-processor
- **Radio**: Digi XBee (802.15.4 / Zigbee) in API mode
- **Cloud**: ThingSpeak (HTTP REST ingest)
- **Build**: Pico SDK (CMake) for the Pico targets; Arduino IDE for the Arduino/ESP targets
- **Hardware design**: KiCad schematics for each node

## Repository layout

This repo uses one branch per network component:

| Branch        | Component                                                        |
|---------------|-----------------------------------------------------------------|
| `BaseStation` | ESP8266 base station: WiFi connect + OLED status                |
| `kanfer`      | CircuitPython coordinator that forwards XBee data to ThingSpeak |
| `TempNode`    | Pico thermistor node (C, Pico SDK)                              |
| `LightNode`   | Arduino luminosity node                                         |
| `XBeeAPI`     | Standalone XBee API frame library + reference firmware          |
| `SerialReader`| Host-side C tool that decodes XBee frames from a serial port    |
| `Diagrams`    | KiCad schematics and the data-flow diagram                      |

## Getting Started

### Prerequisites

- A pair (or more) of Digi XBee radios configured in **API mode**, one as coordinator
- Target boards: Raspberry Pi Pico and/or Arduino-class board for sensor nodes; an ESP8266/Feather for the coordinator
- [Pico SDK](https://github.com/raspberrypi/pico-sdk) + CMake (Pico targets) and/or the Arduino IDE
- Python 3 with the `digi-xbee` library for the host receive tool

### Build a Pico sensor node

```bash
# from the TempNode/ (or XBeeAPI/) directory
mkdir build && cd build
cmake ..
make
# flash the resulting .uf2 to the Pico
```

### Flash an Arduino node / coordinator

Open `LightNode/LightNode.ino` or `BaseStation/BaseStation.ino` in the Arduino IDE and upload to the board.

For the ESP8266 base station, create `BaseStation/WiFiCredentials.h` (git-ignored):

```c
const char* ssid = "your-ssid";
const char* pwd  = "your-password";
```

### Run the host receive tool

```bash
pip install digi-xbee
python ReceiveData.py   # edit PORT/BAUD_RATE to match your XBee
```

## License

Unlicensed (academic project).

## Authors

Jacob Kanfer — [GitHub](https://github.com/Technical-1) — and project partner. Two-person final project for EEL4599.
