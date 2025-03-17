import cv2
import os
import time

RESOLUTION = (1920, 1080)
SAVE_FOLDER = "bombusbox_images_"+time.strftime("%Y-%m-%d_%H-%M-%S")

if not os.path.exists(os.getcwd() + "/" + SAVE_FOLDER):
    os.makedirs(os.getcwd() + "/" + SAVE_FOLDER)

def take_picture(camera):
    ret, frame = camera.read()
    if ret:
        cv2.imshow('Camera View', frame)
        cv2.imwrite(SAVE_FOLDER + "/" + time.strftime("%Y%m%d-%H%M%S") + ".jpg", frame)
        print("Picture taken")
    else:
        print("Error taking picture")

def main():
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
    #cv2.destroyAllWindows()
    return

if __name__ == "__main__":
    main()