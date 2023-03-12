#ifndef BRICK_H
#define BRICK_H

#include "globals.h"
#include "ball.h"

enum bstate { normal,breaking,weakening,invalid};

typedef struct BRICK_T
{
	int health;
	int type;
	int powerup;
	int texture;
	bstate state;
	int timer;
}BRICK;

#define BRICK_NUM_COLORS 11
#define WEAKENING_TEXTURE 10

int drawBrick(BRICK * b,int,int,SDL_Surface* lpddsback);
int drawBrickShadow(BRICK * b,int,int,SDL_Surface* ,SDL_Surface*);
int hitBrick(BRICK bricks[BHEIGHT][BWIDTH],int x,int y);
int hitTheBricks(BRICK bricks[BHEIGHT][BWIDTH],BALL * ball);
int createBrickTexture(SDL_Surface* lpdds,int type,
					  int r,int g,int b);    // transparency flag



#endif
