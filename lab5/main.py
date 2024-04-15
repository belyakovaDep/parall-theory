from ultralytics import YOLO
from queue import Queue
import cv2
import time
import threading
import argparse
import os

que = Queue()

class Cap:
    def __init__(self, name: str):
        self._capture = cv2.VideoCapture(name)

    def __del__(self):
        self._capture.release()

    def get(self):
        ret, frame = self._capture.read()
        return ret, frame

class VideoWrite:
    def __init__(self, name: str):
        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        self._stdout = cv2.VideoWriter(name, fourcc, 29.97, (640, 480))

    def __del__(self):
        self._stdout.release()

    def write(self, frame):
        self._stdout.write(frame)


def for_thread(model, size, start, video, maxSize, i_thread):
    results = []
    end = 0
    for i in range(start, start + size, 1):
        if i == maxSize:
            break
        result = model(video[i])
        res = result[0]
        keypoints = res.keypoints
        results.append(res.plot())
        end = i
    
    que.put([i_thread, results])
    return end

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Args: --filename, --output, --mono or --multy & --number")
    parser.add_argument('--name', type=str, help='Name of input file')
    parser.add_argument('--output', type=str, help='Name of output file')
    parser.add_argument('--mode', choices=['mono', 'multy'], help='Info abt parallelism')
    parser.add_argument('--count', type=int)

    args = parser.parse_args()
    if args.mode == 'multy' and args.count < 2:
        parser.error("Arg '--count' is needed and should be >= 2")
    elif args.mode == 'mono' and args.count is not None:
        parser.error("You can't change the number of threads in mono-mode.")

    if not os.path.exists(args.name):
        print('Error: Can\'t find the input file. Please check the correctness of arguments.')
        exit(1)
    if args.mode == 'mono':
        numOfThreads = 1
    else:
        numOfThreads = args.count

    cap = Cap(args.name)

    video = []
    models = []
    resultss = []
    sizes = []
    threads = []

    out = VideoWrite(args.output)

    while True:
        r, frame = cap.get()
        if not r:
            break
        video.append(frame)

    lens = len(video)

    for i in range(0, numOfThreads):
        model = YOLO('yolov8n-pose.pt')
        models.append(model)
        sizes.append(lens//numOfThreads)
        if lens%numOfThreads != 0 and i < lens%numOfThreads:
            sizes[i] += 1

    start = 0
    times = time.time()
    for i in range(len(models)):
        th = threading.Thread(target=for_thread, args=(models[i], 
                              sizes[i], start, video, lens, i,), daemon=True)
        start += sizes[i]
        threads.append(th)

    times = time.time()
    for th in threads:
        th.start()
    for th in threads:
        th.join()
    ts = time.time()
    print('The count of threads:', numOfThreads, '\nThe worktime is:', ts - times)

    outputs = [[] for i in range(que.qsize())]
    for i in range(que.qsize()):
        res = que.get()
        outputs[res[0]] = res[1]

    for arr in outputs:
        for frame in arr:
            out.write(frame)