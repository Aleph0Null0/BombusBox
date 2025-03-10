import cv2
import os
import time

RESOLUTION = (1920, 1080)
SAVE_FOLDER = "bombusbox_images_"+time.strftime("%Y%m%d-%H%M%S")

if not os.path.exists(os.getcwd() + "/" + SAVE_FOLDER):
    os.makedirs(os.getcwd() + "/" + SAVE_FOLDER)

def take_picture(camera):
    ret, frame = camera.read()
    if ret:
        cv2.imwrite(SAVE_FOLDER + "/" + time.strftime("%Y%m%d-%H%M%S") + ".jpg", frame)
        print("Picture taken")
    else:
        print("Error taking picture")