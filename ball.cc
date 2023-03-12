#include "ball.h"
#include "paddle.h"
#include "font.h"

extern BALL balls[MAX_BALLS];
extern BALL * ballHead;

BALL * getBall()
{
	int i=0;
	while(i<MAX_BALLS)
	{
		if(balls[i].speed==0)
		{
			return &balls[i];
		}
		i++;
	}

	return NULL;
}
int moveBallAmount(BALL * b,float speed);
void moveBall(BALL * b)
{
	if(b==NULL)
		return;
	float flr = (float)floor(b->speed);
	static int wonky_move=0;
	if(b->wonky)
	{
		b->wonky--;
		
                b->dir.x+=b->dir.x*(float)(rand())*((rand()%2==0)?0.5f:-0.5f);
			//b->dir.y+=b->dir.y*(float)(rand())*(rand()%2==0)?1.0f:-1.0f;
			normalize(&b->dir);

	}
	
	while(flr>0)
	{
		if(!moveBallAmount(b,1.0f))
			break;
		flr--;
	}
	if(b->next)
	{
		moveBall(b->next);
	}
}
int moveBallAmount(BALL * b,float speed)
{
	//if(b->next)
	//	moveBall(b->next);
	if(b->stuckTo)
	{
		b->loc.x=((PADDLE*)b->stuckTo)->loc.x+
				 ((PADDLE*)b->stuckTo)->width*8-
				 BALL_RAD;
		return 1;
	}
	b->loc.x+=(b->dir.x*speed);
	b->loc.y+=(b->dir.y*speed);
	if(b->loc.x<=BOUND_LEFT)
	{
		b->dir.x=-b->dir.x;
	}
	if(b->loc.y<=BOUND_UP)
	{
		b->dir.y=-b->dir.y;
	}
	if((b->loc.x+16) >= BOUND_RIGHT)
	{
		b->dir.x=-b->dir.x;
		b->loc.x=BOUND_RIGHT-16;
		//ball->speed=rand()%5+1;
	}
	if((b->loc.y+16)>= BOUND_DOWN)
	{
		BALL * bp= ballHead;
		BALL * previous=NULL;
		b->speed=0;

		while(bp)
		{
			if(bp == b)
			{
				if(bp==ballHead)
				{
					ballHead = b->next;
				}
				else
				{
					if(previous==NULL)
					{
						ballHead = bp->next;
					}
					else
					{
						previous->next = b->next;
					}
				}
				break;
			}
			previous = bp;
			bp = bp->next;
		}
		return 0;
	}
	return 1;
}

void initBall(BALL * b, int x, int y)
{
	b->dir.x=(float) (rand()%100) - 50;
	b->dir.y=-1.0f*(rand()%100+10);
	normalize(&b->dir);
	b->speed=5.0f;
	b->loc.x=(float)x;
	b->loc.y=(float)y;
	b->next=NULL;
	b->powerBall=false;
	b->frame=0;
	b->wonky=0;
}

void stickBallPaddle(BALL * b, void * paddle)
{
	PADDLE * p = (PADDLE *) paddle;
	b->stuckTo = paddle;
	b->loc.x = p->loc.x+8*p->width-BALL_RAD;
	b->loc.y = p->loc.y - BALL_DIAM;
}

int freeBall(BALL * b)
{
	return 1;
}
