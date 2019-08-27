//
#include "probe.h"
#include "ppoint.h"
#include <stdio.h>
//
//void probe_for_ppoint(uberprobe_t *probe, ppoint_t *ppoint) {
//  probe->probe = ppoint;
//  probe->start = (size_t (*)(void *)) ppoint_start;
//  probe->stop = (size_t (*)(void *, size_t)) ppoint_stop;
//}


typedef struct {
    ppoint_t *point;
    size_t start;
} throughput_data_t;

typedef struct {
    ppoint_t *enter;
    ppoint_t *exit;
    size_t enter_start;
    size_t exit_start;
} latency_data_t;


void uberprobe_start(uberprobe_t *probe) {
    probe->start(probe);
}

// interval is nanos
double uberprobe_stop(uberprobe_t *probe, double interval) {
    return probe->stop(probe, interval);
}

int uberprobe_less_than(uberprobe_t *probe, double a, double b) {
    return probe->less_than(a, b);
}


static void uberprobe_throughput_start(uberprobe_t *probe) {
    throughput_data_t *x = (throughput_data_t *)(probe->arg);
    x->start = ppoint_start(x->point);
}

// returns ops / sec
static double uberprobe_throughput_stop(uberprobe_t *probe, double interval) {
    throughput_data_t *x = (throughput_data_t *)(probe->arg);
    return ((double)ppoint_stop(x->point, x->start)) / (interval / (1000 * 1000 * 1000));
}

static int uberprobe_throughput_less_than(double a, double b) {
    return a < b;
}

void throughput_probe_init(uberprobe_t *probe, ppoint_t *point) {
    throughput_data_t *x = (throughput_data_t *)malloc(sizeof(throughput_data_t));
    x->point = point;
    probe->arg = x;
    probe->start = uberprobe_throughput_start;
    probe->stop = uberprobe_throughput_stop;
    probe->less_than = uberprobe_throughput_less_than;
}

static void uberprobe_latency_start(uberprobe_t *probe) {
    latency_data_t *x = (latency_data_t *)(probe->arg);
    x->enter_start = ppoint_start(x->enter);
    x->exit_start = ppoint_start(x->exit);
}

// returns sec
static double uberprobe_latency_stop(uberprobe_t *probe, double interval) {
    latency_data_t *x = (latency_data_t *)(probe->arg);
    size_t enters = ppoint_stop(x->enter, 0);
    size_t exits = ppoint_stop(x->exit, 0);
    
    double arrivals_during_interval = enters - x->enter_start;
    double arrival_rate = (arrivals_during_interval / interval) * 1000 * 1000 * 1000;
    
    // lam = L/rate
    double latency = ((double)(enters - exits)) / arrival_rate;
    
//    printf("# interval=%g enter_start=%lu exter=%lu exits=%lu arrivals=%g arrival_rate=%g latency=%g\n",
//           interval/ (1000 * 1000 * 1000),
//           x->enter_start,
//           enters,
//           exits,
//           arrivals_during_interval,
//           arrival_rate,
//           latency);
    return latency;  // lower latency => return bigger number
}

static int uberprobe_latency_less_than(double a, double b) {
    return a > b;
}

void latency_probe_init(uberprobe_t *probe, ppoint_t *enter, ppoint_t *exit) {
    latency_data_t *x = (latency_data_t *)malloc(sizeof(latency_data_t));
    x->enter = enter;
    x->exit = exit;
    ppoint_start(x->enter);
    ppoint_start(x->exit);
    probe->arg = x;
    probe->start = uberprobe_latency_start;
    probe->stop = uberprobe_latency_stop;
    probe->less_than = uberprobe_latency_less_than;
}

void uberprobe_destroy(uberprobe_t *probe) {
    free(probe->arg);
}





