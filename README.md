
# AdaptiveSync Framework

##### Stephen Freund (Williams College)
##### Developed under NSF Grant 1439042

## Installation

You will need to build the gflags library and place the lib and include directories for it in the gflags directory:

```
 gflags/
   include/ 
   lib/
 AdaptiveSync
   README.md
   Makefile
   ...
```

Then `cd AaptiveSync` and type `make`.

## Examples

- Locks: `cd AdaptiveSync/combine-test` and run `test-1.py` to run an
  adaptive mutex / spinlock test.  The `combine.c` file in that directory demonstrates
  how to build a combined adaptive lock.

- Thread Pool: `cd AdaptiveSync/tpool` and run `test.exe` and
`ubertest.exe` to observe the difference between a standard thread
pool and an adaptive one that rebalances threads.  The thread pool
code is based on the threadpool in the ferret benchmark.



