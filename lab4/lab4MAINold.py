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
        time.sleep(self._delay)
        cv2.imshow('Camera', frame)

class SensorCam():
    def __init__(self, cameraName, resolution):
        self.cap = cv2.VideoCapture(cameraName)
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, resolution[0])
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT,resolution[1])

    def __del__(self):
        self.cap.release()

    def get(self):
        _, frame = self.cap.read()
        return frame

def putInfo(sensor, number: int):
    while True:
        if(number == 4):
            que.queue.insert(0, [number, sensor.get()])
        else:
            que.put([number, sensor.get()])
######################################################3

que = Queue()

if __name__ == "__main__":

    if not os.path.exists("./log"):
        os.makedirs("./log")

    parser = argparse.ArgumentParser(description="Args parser: camName, camRes, camRate")
    parser.add_argument("CamName", type=int)
    parser.add_argument("CamResolution", type=str)
    parser.add_argument("CamRate", type=float)
    args = parser.parse_args()
    camRes = args.CamResolution.split("x")
    camRes = list(map(int, camRes))

    logger = logging.basicConfig(filename = "./log/errors.log", level = logging.ERROR, 
                                 format = '%(asctime)s - %(levelname)s - %(message)s')

    sens1 = SensorX(1)
    sens2 = SensorX(0.1)
    sens3 = SensorX(0.01)

    thread1 = threading.Thread(target=putInfo, args=(sens1, 1,), daemon=True)
    thread2 = threading.Thread(target=putInfo, args=(sens2, 2,), daemon=True)
    thread3 = threading.Thread(target=putInfo, args=(sens3, 3,), daemon=True)

    try:
        cap = SensorCam(args.CamName, camRes)
    except Exception as e:
        print(logging.error(f'Camera Name or Resolution error: {str(e)}'))
        exit(1)

    thread4 = threading.Thread(target=putInfo, args=(cap, 4,), daemon=False)

    show = WindowImage(args.CamRate)

    thread4.start()
    thread1.start()
    thread2.start()
    thread3.start()

    info = [0, 0, 0]
    frame = None
    while(True):
        try:
            if not que.empty():
                inf = que.get_nowait()
                if inf[0] == 1:
                    info[0] = inf[1]
                elif inf[0] == 2:
                    info[1] = inf[1]
                elif inf[0] == 3:
                    info[2] = inf[1]
                else:
                    frame = inf[1]
            
            if frame is not None:
                cv2.putText(frame, 'Sensor0: '+ str(info[0]), (30, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
                cv2.putText(frame, 'Sensor1: '+ str(info[1]), (30, 60), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
                cv2.putText(frame, 'Sensor2: '+ str(info[2]), (30, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
                show.show(frame)
        
        except Exception as e:
            print(logging.error(f'Camera got disconnected: {str(e)}'))
            exit(1)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
                break

## python .\lab4MAIN.py 0 1020x920 0.001