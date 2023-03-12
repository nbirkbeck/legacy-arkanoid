#include "pup.h"

extern POWERUP pups[10];

void initPup(POWERUP * p)
{
	p->speed = 2.0f;
	p->type = PUP_STICKY;
    p->active = 0;
	p->dir.x=0.0f;
	p->accel.x=0.3f;
	p->accel.y=4.0f;
	normalize(&p->accel);
	p->dir.y=1.0f;
	p->loc.x=0;
	p->loc.y=0;
}

void movePup(POWERUP * p)
{
	if(p->active)
	{
		if(p->dir.x<-0.76f)
		{
			p->dir.x=-0.75f;
			normalize(&p->dir);
			p->accel.x=-1*p->accel.x;
		}
		else if(p->dir.x<0.76f)
		{
			p->dir.x+=p->accel.x;
			normalize(&p->dir);
		}
		else
		{
			p->dir.x=0.75f;
			normalize(&p->dir);
			p->accel.x=-1*p->accel.x;
		}
		p->loc.x+=(0.8f*p->dir.x);
		p->loc.y+=(p->speed*p->dir.y);

	}

	if(p->loc.y>480)
		p->loc.y=0;
	if(p->loc.x<0 ||p->loc.x>640)
		p->loc.x=50;
}