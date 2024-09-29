import os
import cv2
import time
import signal 
from PIL import Image
from pathlib import Path
from ultralytics import YOLO

# 함수 정의 
def sigusr1_handler(signum, frame):
    for image_file in image_files:

        if not os.path.isfile(image_file):
            os.kill(ppid, signal.SIGUSR1);
            exit()

        img = Image.open(image_file)

        results = model(img)
        result = results[0]

        if len(result) > 0 and len(result[0].boxes.xyxy) > 0:
            output_path = os.path.join("img", Path(image_file).stem + '.txt')
            print(output_path)

            with open(output_path, 'w') as f:
                for box in result[0].boxes.xyxy.cpu().numpy():
                    xmin, ymin, xmax, ymax = box  # Unpack the coordinates
                    f.write("{:.2f} {:.2f}\n".format((xmax-xmin)/2.0, (ymax-ymin)/2.0))

    with open("main_process.txt", "r") as f:
        pid_str = f.read()
        os.kill(int(pid_str), signal.SIGUSR1)



model = YOLO("ai/fire.pt")
image_files = ["img/left.jpg", "img/right.jpg", "img/front.jpg"]

pid = os.getpid()
signal.signal(signal.SIGUSR1, sigusr1_handler)

with open("ai_process.txt", "w") as f:
    f.write(str(pid))

for image_file in image_files:

    if not os.path.isfile(image_file):
        os.kill(ppid, signal.SIGUSR1);
        exit()

    img = Image.open(image_file)

    results = model(img)
    result = results[0]

    if len(result) > 0 and len(result[0].boxes.xyxy) > 0:
        output_path = os.path.join("img", Path(image_file).stem + '.txt')
        print(output_path)

        with open(output_path, 'w') as f:
            for box in result[0].boxes.xyxy.cpu().numpy():
                xmin, ymin, xmax, ymax = box  # Unpack the coordinates
                f.write("{:.2f} {:.2f}\n".format((xmax-xmin)/2.0, (ymax-ymin)/2.0))


while(1):
    time.sleep(1)
