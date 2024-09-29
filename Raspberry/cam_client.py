import os
import socket
import time
from threading import Thread

import cv2
import numpy as np


# 서버 설정
SERVER_IP = "10.10.141.31" # Server IP
SERVER_PORT = 5000          # Server port
YOLO_PORT = 5022            # Python YOLO port

TIMER_DURATION = 0.5

is_running = True
cam_data = None


# 서버 연결
def connect_to_server(port):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(30)
        sock.connect((SERVER_IP, port))
        return sock
    except Exception as e:
        print(f"[Error] Failed to connect {SERVER_IP}:{port} : {e}")
        return None


def camera_capture_thread():
    global is_running, cam_data

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        return

    os.system('v4l2-ctl -d /dev/video0 --set-ctrl=auto_exposure=1')  # manual exposure
    os.system(f'v4l2-ctl -d /dev/video0 --set-ctrl=exposure_time_absolute=150')
    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]

    while is_running:
        # read camera
        ret, frame = cap.read()
        if not ret:
            break

        # encode frame to jpg format
        result, frame = cv2.imencode('.jpg', frame, encode_param)
        data = np.array(frame)
        byteData = data.tobytes()

        cam_data = str(len(byteData)).encode().ljust(16) + byteData

    cap.release()


def send_image_thread(sock, port):
    global is_running, cam_data

    current_time = 0
    last_time = 0

    try:
        if sock is None:
            sock = connect_to_server(port)

        while is_running:
            try:
                # Check timer
                current_time = time.time()
                if current_time - last_time < TIMER_DURATION:
                    continue
                last_time = current_time

                # data receive
                if cam_data is None:
                    continue
                send_data = cam_data

                # send data
                sock.sendall(send_data)
            except BrokenPipeError:
                print(f"[Error] Disconnected from port {port}. Try reconnecting...")
                sock.close()
                sock = connect_to_server(port)
                if sock is not None and port == SERVER_PORT:
                    response = server_sock.recv(1024).decode("utf-8")
                    print(f"[Info] {response}\n")
                elif sock is None:
                    break
            except Exception as e:
                print(f"[Disconnect] {e}")
                break
    except Exception as e1:
        print(f"[Error] {e1}")
    finally:
        if sock is not None:
            sock.close()
        if port == SERVER_PORT:
            is_running = False


if __name__ == "__main__":
    # connect to server
    server_sock = connect_to_server(SERVER_PORT)
    if server_sock:
        response = server_sock.recv(1024).decode("utf-8")
        print(f"[Info] {response}\n")
    else:
        print("[Error] Failed to connect server. Exit...")

    # start camera thread
    cam_thread = Thread(target=camera_capture_thread)
    cam_thread.daemon = True
    cam_thread.start()

    # start server thread
    server_thread = Thread(target=send_image_thread, args=(server_sock, SERVER_PORT,))
    server_thread.daemon = True
    server_thread.start()

    # start yolo thread
    yolo_sock = None
    yolo_thread = Thread(target=send_image_thread, args=(yolo_sock, YOLO_PORT,))
    yolo_thread.daemon = True
    yolo_thread.start()

    server_thread.join()
    yolo_thread.join()

