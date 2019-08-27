
#ifndef _FLAGS_H_
#define _FLAGS_H_

#include "gflags/gflags.h"

DECLARE_uint64(readDelay);
DECLARE_uint64(writeDelay);
DECLARE_uint64(outsideDelay);

DECLARE_double(writePercent);

DECLARE_uint64(warmup);
DECLARE_uint64(measure);
DECLARE_uint64(wrapup);

DECLARE_uint64(expFreq);
DECLARE_uint64(expTime);


DECLARE_int64(threads);

DECLARE_bool(pp);
DECLARE_bool(quiet);

DECLARE_string(lock);
DECLARE_string(load);

#endif
