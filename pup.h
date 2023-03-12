#ifndef PUP_H
#define PUP_H

#include "globals.h"

typedef struct POWERUP_T {
  VEC2 dir;
  VEC2 loc;
  VEC2 accel;
  int active;
  int type;
  float speed;
} POWERUP;

void initPup(POWERUP* p);
void movePup(POWERUP* p);

#endif