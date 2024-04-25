import board
import busio

uart = busio.UART(tx=board.GP0, rx=board.GP1, baudrate=9600)

def xbee_api_receive_data():
    payload = []
    count = 0
    length = 0
    address_64bit = 0
    address_16bit = 0
    payloadIdx = 0
    while True:
        temp = uart.read(1)
        if count == 0:
            # Wait until start of frame
            if temp[0] != 0x7E:
                continue
        elif count == 1:
            length = int.from_bytes(temp, 'big')
        elif count == 2:
            length = length * 256 + int.from_bytes(temp, 'big')
            payload = [0] * (length - (1 + 8 + 2))
            print(length)
        elif count == 3:
            pass
        elif count < 12:
            address_64bit = address_64bit * 256 + int.from_bytes(temp, 'big')
        elif count < 14:
            address_16bit = address_16bit * 256 + int.from_bytes(temp, 'big')
        elif (count + 1 == (length + 4)):
              break
        else:
              payload[payloadIdx] = temp[0]
              payloadIdx += 1
        
        count += 1
    # Don't verify checksum, assume it is correct
    
    return (payload, address_64bit)

(payload, address) = xbee_api_receive_data()
