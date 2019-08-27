#ifndef PROBE_H_
#define PROBE_H_

#include <stdlib.h>
#include "ppoint.h"

#ifdef __cplusplus
extern "C" {
#endif
 
  
typedef struct uberprobe_a {
  void *arg;
  void (*start)(struct uberprobe_a *);
  double (*stop)(struct uberprobe_a *, double interval);  // interval is nanos
  int (*less_than)(double a, double b);  

} uberprobe_t;

  void uberprobe_start(uberprobe_t *probe);
  double uberprobe_stop(uberprobe_t *probe, double interval); // interval is nanos
  int uberprobe_less_than(uberprobe_t *probe, double a, double b);
  
  void throughput_probe_init(uberprobe_t *probe, ppoint_t *point);
  void latency_probe_init(uberprobe_t *probe, ppoint_t *entry, ppoint_t *exit);
  
  void uberprobe_destroy(uberprobe_t *probe);

#ifdef __cplusplus
}
#endif

#endif
