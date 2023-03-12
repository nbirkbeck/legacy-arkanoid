#ifndef PADDLE_H
#define PADDLE_H

#include "brick.h"
#include "globals.h"
#include "pup.h"

#define MAX_LASERS 100

typedef struct LASER_T {
  VEC2 loc;
  float speed;
  LASER_T* next;
  bool active;
} LASER;

typedef struct PADDLE_T {
  VEC2 dir;
  VEC2 loc;
  int width;
  int sticky;
  int pups;
  LASER* laserHead;
  int lives;
  int lasers;
  int laserReloadedIn;
  SDL_Surface* textures[5];
  SDL_Surface* bullet;
} PADDLE;

int initPaddle(PADDLE* p);
int setPaddleTextures(PADDLE* p, SDL_Surface* l, SDL_Surface* m,
                      SDL_Surface* r);
int setPaddleLasers(PADDLE* p, SDL_Surface* l, SDL_Surface* r,
                    SDL_Surface* bul);
LASER* moveLasers(LASER*, BRICK bricks[BHEIGHT][BWIDTH]);
int setPaddleX(PADDLE* p, int x);
int movePaddle(PADDLE* p, bool keys[256]);
int movePaddleLasers(PADDLE* p, BRICK bricks[BHEIGHT][BWIDTH]);
int setPaddleWidth(PADDLE* p, int w);
int freePaddle(PADDLE* p);
int checkForCollision(PADDLE* p, BALL* b);
int checkForCollisionPup(PADDLE* p, POWERUP* b);

void initLaser(LASER* l);

#endif
