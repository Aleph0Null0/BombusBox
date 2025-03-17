import serial
import time
from datetime import datetime


def log_serial_data(port='COM3', baud_rate=9600):

    filename = f'sensor_data_{datetime.now().strftime("%Y%m%d_%H%M%S")}.csv'

    ser = serial.Serial(port, baud_rate, timeout=1)
    print(f"Connected to {port}")

    try:
        with open(filename, 'w') as file:
            while True:
                if ser.in_waiting:
                    line = ser.readline().decode('utf-8').strip()
                    print(line)  # Display in console
                    file.write(line + '\n')
                    file.flush()  # Ensure data is written immediately

    except KeyboardInterrupt:
        print("\nLogging stopped by user")
    finally:
        ser.close()


if __name__ == "__main__":
    # You may need to change the port name
    # Windows: 'COM3', 'COM4', etc.
    # Linux/Mac: '/dev/ttyUSB0', '/dev/ttyACM0', etc.
    log_serial_data(port='COM3')