import serial
import time
from datetime import datetime
from threading import Thread, Event
import cv2
import os

RESOLUTION = (1920, 1080)
SAVE_FOLDER = "bombusbox_images_"+time.strftime("%Y-%m-%d_%H-%M-%S")

if not os.path.exists(os.getcwd() + "/" + SAVE_FOLDER):
    os.makedirs(os.getcwd() + "/" + SAVE_FOLDER)

def take_picture(camera):
    ret, frame = camera.read()
    if ret:
        #cv2.imshow('Camera View', frame)
        cv2.imwrite(SAVE_FOLDER + "/" + time.strftime("%Y%m%d-%H%M%S") + ".jpg", frame)
        print("Picture taken")
    else:
        print("Error taking picture")



def logData(port='COM3', baud_rate=9600):
    filename = f'sensor_data_{datetime.now().strftime("%Y%m%d_%H%M%S")}.csv'

    ser = serial.Serial(port, baud_rate, timeout=1)
    print(f"Connected to {port}")

    running = Event()
    running.set()
    def sendTime():
        hour = datetime.now().hour
        prev_hour = hour
        while running.is_set():
            hour = datetime.now().hour
            if True:
                ser.write(bytes(hour))
    def picture_loop():
        camera = cv2.VideoCapture(0)
        camera.set(cv2.CAP_PROP_FRAME_WIDTH, RESOLUTION[0])
        camera.set(cv2.CAP_PROP_FRAME_HEIGHT, RESOLUTION[1])
        if not camera.isOpened():
            print("Error opening camera")
        while True:
            take_picture(camera)
            if cv2.waitKey(5000) == ord('q'):
                break

        camera.release()
        return

    #time_thread = Thread(target=sendTime)
    #time_thread.daemon = True
    #picture_thread = Thread(target=picture_loop)
    #picture_thread.daemon = True

    try:
        with open(filename, 'w') as file:
            #time_thread.start()
            #picture_thread.start()
            while running.is_set():
                if ser.in_waiting:
                    line = ser.readline().decode('utf-8').strip()
                    print(line)  # Display in console
                    file.write(line + '\n')
                    file.flush()  # Ensure data is written immediately

    except KeyboardInterrupt:
        print("\nLogging stopped by user")
    except:
        print("Something went wrong.")
    finally:
        running.clear()  # Signal threads to stop
        #time_thread.join(timeout=2)
        #picture_thread.join(timeout=2)
        ser.close()


if __name__ == "__main__":
    # Windows: 'COM3', 'COM4', etc.
    # Linux/Mac: '/dev/ttyUSB0', '/dev/ttyACM0', etc.
    # /dev/tty.usbserial-B0004ZWB
    logData(port='/dev/tty.usbserial-B0004ZWB')
