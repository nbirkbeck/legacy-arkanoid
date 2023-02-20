
#ifndef GLOBALLS
#define GLOBALLS
#include "compat.h"
#include <math.h>
#include <stdio.h>

#define LEFT   0
#define MIDDLE 1
#define RIGHT  2
#define LEFT_LASER 3
#define RIGHT_LASER 4

#define LASER_SPEED 7

#define BOUND_LEFT 96
#define BOUND_RIGHT (SCREEN_WIDTH - 96)
#define BOUND_UP 32
#define BOUND_DOWN (SCREEN_HEIGHT - 32)

#define BRICK_NUM_TYPES 7

#define BRICK_SOLO 0
#define BRICK_LEND 1
#define BRICK_REND 2
#define BRICK_HOR  3
#define BRICK_VER  4
#define BRICK_TEND 5
#define BRICK_BEND 6

#define BHEIGHT 15
#define BWIDTH  28

#define BALL_DIAM 16
#define BALL_RAD 8

#define NUM_PUPS 13
#define PUP_STICKY 0
#define PUP_WIDTH  1
#define PUP_LASERS 2
#define PUP_BALL   3
#define PUP_POWER_BALL   4
#define PUP_MEGA_BALL 5
#define PUP_MEGA_LASERS 6
#define PUP_WARP   7
#define PUP_LIFE   8
#define PUP_SHRINK  9
#define PUP_WONKY  10
#define PUP_REVERSE 11
#define PUP_DEATH  12


#define SCREEN_WIDTH 640  // 96//640 // size of screen
#define SCREEN_HEIGHT 480 // 160//480

#define MAX_PUPS 20

#define BDBG 1


typedef struct VEC2_T
{
	float x,y;
}VEC2;

int normalize(VEC2 * v);
int Level_Intro();


#endif
