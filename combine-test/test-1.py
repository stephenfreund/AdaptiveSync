#!/usr/bin/python

import subprocess


for writePercent in [0, 0.001, 0.01, 0.1, 0.2, 1] :
    for writeDelay in [0,100,500,1000]:
        for readDelay in [writeDelay]:
            for threads in [4,8]:
                for lock in ['Mutex','Spin','SM']:
                    cmd = "./exp.exe --quiet --lock=%s --writePercent=%g -writeDelay=%d -readDelay=%d --measure=5 --threads=%d -expFreq=1000" % (lock, writePercent, writeDelay, readDelay, threads)
                    p = subprocess.Popen(cmd, shell=True)
                    retval = p.wait()
