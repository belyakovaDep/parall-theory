I'm  really sorry for using the video without me, but it's a huge cringe to see myself while I'm debugging, so here You can see Chloe Tang. :D

to run:
    python ./main.py --name=input.mp4 --output=out.avi --mode=multy --count=3
or
    python ./main.py --name=input.mp4 --output=out.avi --mode=mono

1 thread:
    11.4s
2 threads:
    11.9s
3 threads:
    11.7
4 threads:
    12.46
5 threads:
    12.1
6 threads:
    12.4
8 threads:
    11.9
17 threads:
    13.3

Conclusion:
    The best number of threads: 1. Cos parallelism isn't a really good idea. :D