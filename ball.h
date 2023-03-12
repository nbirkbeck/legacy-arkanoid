

#ifndef BALL_H
#define BALL_H

#include "globals.h"

#define MAX_BALLS 100

typedef struct BALL_T {
  VEC2 dir;
  VEC2 loc;
  float speed;
  bool powerBall;
  void* stuckTo;
  BALL_T* next;
  int frame;
  int wonky;
} BALL;

BALL* getBall();
int freeBall(BALL* b);
void stickBallPaddle(BALL* b, void* paddle);
void initBall(BALL* b, int x, int y);
void moveBall(BALL* b);

#endif