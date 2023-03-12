#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "brick.h"

#include <string.h>

FILE * rdebug = fopen("debugread.txt","w");
extern SDL_Surface*  brickTextures[BRICK_NUM_COLORS][BRICK_NUM_TYPES];
extern int (* Game_Main_Func)(void);
extern char levelName[255];

int readBrick(FILE * file,BRICK * brick)
{
	char str[255];
	char t;
	static int pup=0;
	if(EOF==fscanf(file,"%s %d",str,&(brick->texture)))
		return 0;

	t=str[0];
	fprintf(rdebug,"Read in %s %d",str,(brick->texture));
	brick->health=1;
	brick->state=normal;
	brick->timer=16;
	switch(t)
	{
	case '\"':
		brick->type = BRICK_VER;
		break;
	case '=':
		brick->type = BRICK_HOR;
		break;
	case '>':
		brick->type = BRICK_REND;
		break;
	case '<':
		brick->type = BRICK_LEND;
		break;
	case '^':
		brick->type = BRICK_TEND;
		break;
	case 'v':
		brick->type = BRICK_BEND;
		break;
	case 'o':
		brick->type = BRICK_SOLO;
		break;
	default:
		brick->health=0;
		brick->state=invalid;
		break;
	}
	brick->powerup=-1;
	if(rand()%10==1)
	{

		brick->powerup=pup%MAX_PUPS;
		pup++;
	}
	return 1;
}


int readLevel(const char * filename,BRICK bricks[BHEIGHT][BWIDTH])
{
	int ntexts=0;
	int cnt=0;
	int i=0;
	int j=0;
	static int called=0;
        char full_filename[128];
        snprintf(full_filename, sizeof(full_filename), "levels/%s", filename);
	FILE * file = fopen(full_filename,"r");
	rdebug = fopen("debugread.txt","w");


	if(file==NULL)
		return 0;

	if(!fscanf(file,"%d",&ntexts))
		return 0;
	
	if(called>0)
	{
		
		i=0;
		while(i<BRICK_NUM_COLORS)
		{
			int j=0;
			while(j<BRICK_NUM_TYPES)
			{
				brickTextures[i][j]=NULL;
				j++;
			}
			i++;
		}
	}

	called++;

	while(cnt<ntexts)
	{
		int r,g,b;
		i=0;
		fscanf(file,"%d %d %d",&r,&g,&b);


		while(i<BRICK_NUM_TYPES)
		{
                  brickTextures[cnt][i]=SDL_CreateRGBSurface(0,16,16,32,0,0,0,0);
                  createBrickTexture(brickTextures[cnt][i],i,r,g,b);		
                  i++;
		}
		fprintf(rdebug,"read in color %d\n, it is %d %d %d",cnt,r,g,b);
		cnt++;	
	}
	i=0;
	while(i<BRICK_NUM_TYPES)
	{
		brickTextures[WEAKENING_TEXTURE][i]=SDL_CreateRGBSurface(0,16,16,32,0,0,0,0);
		createBrickTexture(brickTextures[WEAKENING_TEXTURE][i],i,201,220,210);		
		i++;
	}




	memset(bricks,0,sizeof(BRICK)*BHEIGHT*BWIDTH);
	i=0;
	while(readBrick(file,&bricks[i][j]))
	{

		j++;
		fprintf(rdebug,"read in brick %d %d\n",i,j);
		if(j>=BWIDTH)
		{
			j=0;
			i++;
		}
	}
	fclose(file);
	fclose(rdebug);
	const char * lastdot = strrchr(filename,'.');
	if(lastdot==NULL)
          sscanf(filename, "%s", levelName);
	else
	{
		memcpy(levelName,filename,lastdot-filename);
		levelName[lastdot-filename]='\0';
	}
	Game_Main_Func=Level_Intro;

        return 0;
}

