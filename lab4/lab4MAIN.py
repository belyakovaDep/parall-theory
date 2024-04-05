import time
import threading
import cv2
import logging
import argparse

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

def putInfo(sensor: SensorX, sInfo):
    while True:
        sInfo[0] = sensor.get()
######################################################3


if __name__ == "__main__":
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
    sInfo1 = [0]
    sInfo2 = [0]
    sInfo3 = [0]

    thread1 = threading.Thread(target=putInfo, args=(sens1, sInfo1,), daemon=True)
    thread2 = threading.Thread(target=putInfo, args=(sens2, sInfo2,), daemon=True)
    thread3 = threading.Thread(target=putInfo, args=(sens3, sInfo3,), daemon=True)
    

    try:
        cap = SensorCam(args.CamName, camRes)
    except Exception as e:
        logging.error(f'Camera Name or Resolution error: {str(e)}')

    show = WindowImage(args.CamRate)

    thread1.start()
    thread2.start()
    thread3.start()

    while(True):
        try:
            frame = cap.get()
            cv2.putText(frame, 'Sensor0: '+ str(sInfo1[0]), (30, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
            cv2.putText(frame, 'Sensor1: '+ str(sInfo2[0]), (30, 60), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
            cv2.putText(frame, 'Sensor2: '+ str(sInfo3[0]), (30, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
            show.show(frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        
        except Exception as e:
            logging.error(f'Camera got disconnected: {str(e)}')

    cap.__del__()
    show.__del__()

## python .\Untitled-1.py 0 1020x920 0.001