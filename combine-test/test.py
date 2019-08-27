#!/usr/bin/python

import subprocess


for writeDelay in [500,100,1500]:
    for readDelay in [500,100,1500]:
        for threads in [16,4,8]:
            for writePercent in [0.0001, 0.01, 0.025, 0.05, 0.075, 0.1, 0.15, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5 ] :
                for lock in ['Spin', 'SpinRTM', 'RW', 'Mutex', 'MRW', 'SM']:
                    if lock == 'MRW' or lock == 'SM':
                        freqs = [0,500,2000]
                    else:
                        freqs = [0]
                    for expFreq in freqs:
                        for i in range(0,5):
                            cmd = "./exp.exe --quiet --lock=%s --writePercent=%g -writeDelay=%d -readDelay=%d --outsideDelay=%d --warmup=4 --measure=5 --threads=%d --expFreq=%d --expTime=10" % (lock, writePercent, writeDelay, readDelay, 0, threads, expFreq)
                            p = subprocess.Popen(cmd, shell=True)
                            retval = p.wait()
