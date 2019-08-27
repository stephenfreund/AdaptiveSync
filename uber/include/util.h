/*
 * util.h
 *
 *  Created on: Jan 10, 2017
 *      Author: freund
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

  
// returns nanos
size_t util_get_time(void);
    
// returns nanos
size_t util_wait(size_t ns);
    
// returns millis
size_t util_wait_ms(size_t ms);

void util_log(const char *format, ...);

//#define LOG(X) util_log X
#define LOG(X)

#ifdef __cplusplus
}
#endif

#endif /* UTIL_H_ */
