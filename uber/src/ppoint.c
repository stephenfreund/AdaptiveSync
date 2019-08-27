/*
 * ppoint.c
 *
 *  Created on: Jan 19, 2017
 *      Author: freund
 */

#include "ppoint.h"

void ppoint_init(ppoint_t *p) {
  p->count = 0;
  p->on = 0;
}

static ppoint_t _default_ppoint __attribute__ ((__aligned__(64))) = { 0, 0 };

ppoint_t *default_ppoint() {
  return &_default_ppoint;
}



