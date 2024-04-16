import time
import threading
import cv2
import logging
import argparse
from queue import Queue
import os

##################################################
class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")
    
class SensorX(Sensor):
    def __init__(self, delay: float):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data
    
class WindowImage():
    def __init__(self, delay: int):
        self._delay = delay

    def __del__(self):
        cv2.destroyAllWindows()

    def show(self, frame):
        time.sleep(self._delay/2)
        cv2.imshow('Camera', frame)

class SensorCam():
    def __init__(self, cameraName, resolution):
        self.cap = cv2.VideoCapture(cameraName)
        if not self.cap.isOpened():
             raise Exception('Camera\'s index is wrong.')
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, resolution[0])
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT,resolution[1])

    def __del__(self):
        self.cap.release()

    def get(self):
        ret, frame = self.cap.read()
        # if not ret or frame is None:
        #      raise Exception('Unable to read the input.')
        return frame

def putInfo(sensor, que: Queue):
    while True:
        if que.full():
            _ = que.get_nowait()

        que.put_nowait(sensor.get())
######################################################

queSens1 = Queue(maxsize=2)
queSens2 = Queue(maxsize=2)
queSens3 = Queue(maxsize=2)
queCam = Queue(maxsize=2)

if __name__ == "__main__":

    if not os.path.exists("./log"):
        os.makedirs("./log")

    parser = argparse.ArgumentParser(description="Args parser: Name, Res, Rate")
    parser.add_argument("--name", type=int)
    parser.add_argument("--res", type=str)
    parser.add_argument("--rate", type=float)
    args = parser.parse_args()
    camRes = args.res.split("x")
    camRes = list(map(int, camRes))

    logger = logging.basicConfig(filename = "./log/errors.log", level = logging.ERROR, 
                                 format = '%(asctime)s - %(levelname)s - %(message)s')

    try:
        cap = SensorCam(args.name, camRes)
    except Exception as e:
        print(logging.error(f'Input Device Initialization Error: {str(e)}'))
        exit(1)
    
    sens1 = SensorX(1)
    sens2 = SensorX(0.1)
    sens3 = SensorX(0.01)

    thread1 = threading.Thread(target=putInfo, args=(sens1, queSens1,), daemon=True)
    thread2 = threading.Thread(target=putInfo, args=(sens2, queSens2,), daemon=True)
    thread3 = threading.Thread(target=putInfo, args=(sens3, queSens3,), daemon=True)
    thread4 = threading.Thread(target=putInfo, args=(cap, queCam,), daemon=True)

    show = WindowImage(args.rate)

    thread4.start()
    thread1.start()
    thread2.start()
    thread3.start()

    info = [0, 0, 0]
    frame = None
    
    while not (cv2.waitKey(1) & 0xFF == ord('q')):
        try:
            if not queCam.empty():
                    frame = queCam.get_nowait()

            if not queSens1.empty():
                    info[0] = queSens1.get_nowait()
            
            if not queSens2.empty():
                    info[1] = queSens2.get_nowait()

            if not queSens3.empty():
                    info[2] = queSens3.get_nowait()

            if frame is None:
                raise Exception('Unable to read the input.')
            if frame is not None:
                cv2.putText(frame, 'Sensor0: '+ str(info[0]), (30, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
                cv2.putText(frame, 'Sensor1: '+ str(info[1]), (30, 60), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
                cv2.putText(frame, 'Sensor2: '+ str(info[2]), (30, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
                show.show(frame)
    
        except Exception as e:
            print(logging.error(f'Camera Got Disconnected: {str(e)}'))
            exit(1)
## python .\main.py --name=0 --res=640x480 --rate=0.001