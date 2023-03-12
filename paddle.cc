#include "paddle.h"
#include <SDL2/SDL_keycode.h>

#define PADDLE_DEFAULT_LIVES 3
extern BALL * ballHead;
extern int curBlockCount;
extern int bestBlockCount;

extern BALL * ballHead;

FILE * debug;



void initLaser(LASER * l)
{
	l->loc.x=0;
	l->loc.y=0;
	l->next=NULL;
    l->active = 0;
	l->speed=4;
}


int initPaddle(PADDLE * p)
{
	p->dir.x=p->dir.y=0;
	p->loc.x=280;
	p->loc.y=400;
	p->width=5;
	p->sticky=0;
	p->laserHead=0;
	p->lasers=0;
	p->lives=PADDLE_DEFAULT_LIVES;
	p->laserReloadedIn = 0;
	debug=fopen("debug.txt","w");
	return 1;
}

int setPaddleTextures(PADDLE * p,
					  SDL_Surface* l,
					  SDL_Surface* m,
					  SDL_Surface* r)
{
	p->textures[LEFT]=l;
	p->textures[MIDDLE]=m;
	p->textures[RIGHT]=r;
	return 1;
}

int setPaddleLasers(PADDLE * p,
					SDL_Surface* l,
					SDL_Surface* r,
					SDL_Surface* bul)
{
	p->textures[LEFT_LASER]=l;
	p->textures[RIGHT_LASER]=r;
	p->bullet = bul;
	return 1;
}

int movePaddleLasers(PADDLE * p,BRICK bricks[BHEIGHT][BWIDTH])
{
	p->laserHead = moveLasers(p->laserHead,bricks);
	return 1;
}
/*
	returns the next laser if it has hit something, and
	itself if it hasnt
*/
LASER * moveLasers(LASER * l,BRICK bricks[BHEIGHT][BWIDTH])
{	
	if(l==NULL)
		return NULL;

	l->next=moveLasers(l->next,bricks);

	l->loc.y-=LASER_SPEED;	
	if(hitBrick(bricks,l->loc.x,l->loc.y))
	{
		LASER * ret = l->next;
		l->active=false;
		l->next=NULL;
		return ret;
	}

	if(l->loc.y<BOUND_UP)
	{
		LASER * ret = l->next;
		l->active=false;
		l->next=NULL;
		return ret;
	}
	return l;
	
}

int setPaddleX(PADDLE * p,int x)
{
	float oldx=p->loc.x;
	p->loc.x=x;
	if(x<BOUND_LEFT)
		p->loc.x=BOUND_LEFT;
	if(x>=(BOUND_RIGHT - (p->width<<4)))
		p->loc.x=BOUND_RIGHT - (p->width<<4);
	if(oldx<p->loc.x)
		p->dir.x=-1.0f;
	else if(oldx>p->loc.x)
		p->dir.x=1.0f;
	else p->dir.x=0;
	return 1;
}

int movePaddle(PADDLE * p,bool keys[256])
{
	if(keys[SDLK_RIGHT])
	{
		p->loc.x+=8;
		p->dir.x=-1;
	}
	else if(keys[SDLK_LEFT])
	{
		p->loc.x-=8;
		p->dir.x=1;

	}
	else
	{
		p->dir.x=0;
	}
	if(p->loc.x<BOUND_LEFT)
		p->loc.x=BOUND_LEFT;
	if(p->loc.x>=(BOUND_RIGHT - (p->width<<4)))
		p->loc.x=BOUND_RIGHT - (p->width<<4);
	return 1;
}

int setPaddleWidth(PADDLE * p,int w)
{
	if(w>=2)
	{
		p->width=w;
	}
	return p->width;
}


int checkForCollision(PADDLE *p,BALL *b)
{
	if(b==NULL)
		return 0;
	if(b->next)
		checkForCollision(p,b->next);
	if(b->dir.y<=0)
	{
		return 0;
	}
	if((b->loc.x +BALL_RAD > p->loc.x) && (b->loc.x + BALL_RAD < (p->loc.x + (p->width<<4))))
	{
		if(((b->loc.y + 16)>=p->loc.y) &&
			((b->loc.y+8)<=p->loc.y))
		{
			float r = rand()%10+1;
			b->dir.y*=-1;
			r=-1*r*p->dir.x/10.0f;
			b->dir.x += r;
			normalize(&b->dir);

			if(curBlockCount>bestBlockCount)
				bestBlockCount=curBlockCount;
			curBlockCount=0;

			if(p->sticky)
			{
				b->stuckTo=p;
			}
		}
	}
	else if((b->loc.x +16 >=p->loc.x) && (b->loc.x < p->loc.x) && b->dir.x>0)
	{
		if(((b->loc.y+8)>=p->loc.y) && (p->loc.y<=p->loc.y+8))
		{
			b->dir.x*=-1;
		}
	}
	else if((b->loc.x <= (p->loc.x + (p->width<<4))) && (b->loc.x > p->loc.x) && b->dir.x<0)
	{
		if(((b->loc.y+8)>=p->loc.y) && (p->loc.y<=p->loc.y+8))
		{
			b->dir.x*=-1;
		}
	}
        return 0;
}

int checkForCollisionPup(PADDLE *p,POWERUP *b)
{
	if(b->dir.y<0)
	{
		return 0;
	}
	if((b->loc.x > p->loc.x) && (b->loc.x < (p->loc.x + (p->width<<4))))
	{
		if((b->loc.y + 16)>=p->loc.y)
		{
			if(b->active==0)
				return 0;
			b->active=0;
			
			switch(b->type)
			{
			case PUP_STICKY:
				p->sticky=1;
				p->lasers=0;
				break;
			case PUP_WIDTH:
				p->width++;
				break;
			case PUP_LASERS:
				p->sticky=0;
				p->lasers+=10;
				p->laserReloadedIn=0;
				break;
			case PUP_BALL:
				{
					BALL * bp;
					bp = ballHead;
					if(bp==NULL)
					{
						ballHead=getBall();
						initBall(ballHead,p->loc.x+p->width*8-8,p->loc.y-BALL_RAD);
						return 1;
					}
					while(bp->next)
					{
						bp = bp->next;
					}
					bp->next = getBall();
					if(bp->next!=NULL)
						initBall(bp->next,p->loc.x+p->width*8-8,p->loc.y-BALL_RAD);
				}
				break;
			case PUP_POWER_BALL:
				{
					if(ballHead!=NULL)
					{
						ballHead->powerBall=true;
					}
				}
				break;
			case PUP_MEGA_BALL:
				{
					BALL * tail=ballHead;
					BALL * ball;
					if(tail==NULL)
					{
						ballHead=getBall();
						initBall(ballHead,p->loc.x+(p->width<<3),p->loc.y-BALL_DIAM);
						tail=ballHead;
						tail->dir.x=(float)((float)(rand()%200-100))/0.75f;
					}
					while(tail->next)
					{
						tail = tail->next;
					}
					while((tail->next=getBall())!=NULL)
					{
						tail=tail->next;
						initBall(tail,p->loc.x+(p->width<<3),p->loc.y-BALL_DIAM);
						tail->dir.x=(float)((float)(rand()%200-100))/120.0f;
						normalize(&tail->dir);
					}
				}
				break;
			case PUP_LIFE:
				{
					p->lives++;
				}
				break;
			case PUP_DEATH:
				{
					p->lives--;
				}
				break;
			case PUP_SHRINK:
				{
					
				}
				break;
			case PUP_WONKY:
				{
					ballHead->wonky=200;
					break;
				}
			case PUP_REVERSE:
				{
				}
			case PUP_MEGA_LASERS:
				{
					p->lasers+=100;
					break;
				}
			}
		}
	}
        return 0;
}


int freePaddle(PADDLE * p)
{
	int i=0;
	while(i<5)
	{
		if(p->textures[i])
		{
                  SDL_FreeSurface(p->textures[i]);
			p->textures[i]=NULL;
		}
		i++;
	}
	if(p->bullet)
	{
          SDL_FreeSurface(p->bullet);
          p->bullet=NULL;
	}
	return 1;
}
