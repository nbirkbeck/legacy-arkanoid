
#include "globals.h"

extern int bestBlockCount;
extern int curBlockCount;
extern int score;

int normalize(VEC2 * v)
{
	float len;
	len = v->x*v->x+v->y*v->y;
	len = (float)sqrt(len);
	v->x/=len;
	v->y/=len;
	return 1;
}