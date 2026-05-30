import os
import adafruit_connection_manager
import board
import busio
from adafruit_esp32spi import adafruit_esp32spi, adafruit_esp32spi_wifimanager
import digitalio
import adafruit_requests
from secrets import secrets

# Initialize UART
uart = busio.UART(board.TX, board.RX, baudrate=9600)

# SPI setup
spi = busio.SPI(board.SCK, board.MOSI, board.MISO)
cs = digitalio.DigitalInOut(board.D13)
ready = digitalio.DigitalInOut(board.D11)
reset = digitalio.DigitalInOut(board.D12)
# Initialize ESP32 SPI
esp = adafruit_esp32spi.ESP_SPIcontrol(spi, cs, ready, reset)
# Setup WiFi manager
wifi_manager = adafruit_esp32spi_wifimanager.ESPSPI_WiFiManager(esp, secrets)

# Get WiFi details from secrets file
ssid = secrets["ssid"]
password = secrets["password"]

# Connect to wifi
if not esp.is_connected:
    print("Connecting to WiFi...")
    wifi_manager.connect()

while True:
    # Read from the XBee module
    if uart.in_waiting > 0:
        try:
            data = uart.readline().strip()
            print(data)
            
            # Initialize connection manager
            pool = adafruit_connection_manager.get_radio_socketpool(esp)
            ssl_context = adafruit_connection_manager.get_radio_ssl_context(esp)

            # Initialize SSL context
            session = adafruit_requests.Session(pool, ssl_context)
            # URL
            thingspeak_url = "https://api.thingspeak.com/update?api_key=" + secrets["thingverse-api"]

            # Set field Values
            room_temperature = 25.5  # Example value
            room_illumination = 500  # Example value

            # Add data to get url
            thingspeak_url += f"&field1={room_temperature}&field2={room_illumination}"

            print("Sending data to ThingSpeak...")
            # GET request
            response = session.get(thingspeak_url)
            print("Response:", response.text)

            # Close the response
            response.close()

        except UnicodeError as e:
            print("UnicodeError:", e)

