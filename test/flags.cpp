#include "flags.h"


DEFINE_uint64(readDelay, 100, "Read Delay");
DEFINE_uint64(writeDelay, 100, "Write Delay");
DEFINE_uint64(outsideDelay, 0, "Outside Critical Section Delay");

DEFINE_double(writePercent, 0.01, "Write Percent [0-1]");

DEFINE_uint64(warmup, 1, "Warmup (s)");
DEFINE_uint64(measure, 5, "Measure (s)");
DEFINE_uint64(wrapup, 1, "Wrapup (s)");

DEFINE_uint64(expFreq, 0, "adapt frequency (ms) --- 0 for no adapation");
DEFINE_uint64(expTime, 10, "experiment duration (ms)");


DEFINE_uint64(threads, 8, "Num Threads");

DEFINE_bool(pp, false, "Use Progress Points");
DEFINE_bool(quiet, false, "Quiet");

DEFINE_string(lock, "Mutex", "Lock Type (Mutex/RW/Uber/UberRW)");
DEFINE_string(load, "Basic", "WorkLoad");
