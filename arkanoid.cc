// pacman.cpp : Defines the entry point for the application.
//

#include "globals.h"
#include "compat.h"
#include "MyTimer.h"
#include <stdio.h>
#include "levelreader.h"
#include "font.h"
#include "console.h"
#include "paddle.h"
#include "ball.h"
#include "pup.h"
#include "brick.h"
#include <unordered_map>
#include <iostream>
#include <SDL2/SDL.h>
#include <cassert>

SDL_Window* window = nullptr;

SDL_Surface*  surface = nullptr;   // dd primary surface
SDL_Renderer* renderer = nullptr;
std::unordered_map<int, bool>	keys;

bool fullscreen=false;
bool slow=false;
bool paused=false;
char levelName[255];

SDL_Surface*  test, *test2;
SDL_Surface*  pupTextures[NUM_PUPS];
SDL_Surface*  pupShadowTextures[NUM_PUPS];
SDL_Surface*  brickTextures[BRICK_NUM_COLORS][BRICK_NUM_TYPES];
SDL_Surface*  background;

SDL_Surface*  ballTexture;
SDL_Surface*  ballShadowTexture;
SDL_Surface*  pballTextures[4];


#ifdef ENABLE_SOUND
#define SOUND_CHANNEL 0
FSOUND_SAMPLE * breakSounds[4];
int channel=0;
#endif

PADDLE paddle;
BALL * ballHead;
BALL balls[MAX_BALLS];
LASER lasers[MAX_LASERS];
POWERUP pups[MAX_PUPS];
BRICK bricks[BHEIGHT][BWIDTH];


int bestBlockCount=0;
int curBlockCount=0;
int score=0;

Font font;
Font font12;
//Sounds int chompID=NULL;

int (* Game_Main_Func)(void);
void DrawFPS();


int gcount=0;
int weak;
int scores[2];
int lives[]={2,2};
int multiple=1;

MyTimer timer;
POINT mpos;

int Game_Main();


void  updateScreen()
{
  SDL_UpdateWindowSurface(window);
#ifdef TODO
  if(fullscreen)
    while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));
  else
  {
    RECT rect;
    POINT p1;
    POINT p2;

    ZeroMemory(&rect, sizeof( rect ));

    // get the client area
    GetClientRect(main_window_handle, &rect);

    // copy the rect's data into two points
    p1.x = rect.left;
    p1.y = rect.top;
    p2.x = rect.right;
    p2.y = rect.bottom;

    // convert it to screen coordinates (like DirectDraw uses)
    ClientToScreen(main_window_handle, &p1);
    ClientToScreen(main_window_handle, &p2);

    // copy the two points' data back into the rect
    rect.left   = p1.x;
    rect.top    = p1.y;
    rect.right  = p2.x;
    rect.bottom = p2.y;

    // blit the back buffer to our window's position
    lpddsprimary->Blt(&rect, surface, NULL, DDBLT_WAIT, NULL);
  }
#endif
}

SDL_Surface* createShadow(SDL_Surface* source,int width, int height)
{
  SDL_Surface* src32 = SDL_CreateRGBSurface(0, source->w, source->h, 32, 0,0,0,0);
  SDL_BlitSurface(source, nullptr, src32, nullptr);

  SDL_Surface* shadow = SDL_CreateRGBSurface(0, width,height,32, 0,0,0,0);

	
  // TODO: lock the display surface "source"
  SDL_LockSurface(src32);
  SDL_LockSurface(shadow);

  uint32_t * dest_ptr = (uint32_t*) shadow->pixels;

  int black = 0;
  int shadowColor = SDL_MapRGB(shadow->format, 6,6,6);

  int src_stride =  src32->pitch / 4;
  int dest_stride =  shadow->pitch / 4;
  memset(dest_ptr, 0, 4 * width * height);
  for(int y=0;y<height;y++)
  {
    uint32_t* source_row = (uint32_t*)((char*)src32->pixels + y * src32->pitch);
    for(int x=0;x<width;x++)
    {
      if(source_row[x]!=black)
      {
        dest_ptr[y*dest_stride+x]=shadowColor;
      }
       else dest_ptr[y*dest_stride+x]=black;
    }
  }

  SDL_UnlockSurface(src32);
  SDL_UnlockSurface(shadow);

  SDL_FreeSurface(src32);
  
  SDL_SetColorKey(shadow, 1, 0);
  return shadow;
}


int drawBallShadow(BALL * b)
{
	if(b==NULL)
		return 0;
	drawBallShadow(b->next);
	DDraw_Draw_Surface(ballShadowTexture,(int)b->loc.x+8,(int)b->loc.y+8,16,16,surface,1);
	return 1;
}

int drawBall(BALL * b)
{
	char temp[25];
	if(b==NULL)
		return 0;
#ifdef DRAWBALLINFO
	sprintf(temp,"%4.2f,%4.2f,%4.2f %d",b->dir.x,b->dir.y,b->speed,b->wonky);
	drawFont(surface, &font12,temp,b->loc.x,b->loc.y);
#endif
	if(b->next)
		drawBall(b->next);
	if(b->powerBall)
	{
		b->frame++;
		DDraw_Draw_Surface(pballTextures[((b->frame>>2)%4)],(int)b->loc.x,(int)b->loc.y,16,16,surface,1);
	}
	else
		DDraw_Draw_Surface(ballTexture,(int)b->loc.x,(int)b->loc.y,16,16,surface,1);

	return 1;
}

void movePups()
{
	int i=0;
	while(i<MAX_PUPS)
	{
		movePup(&pups[i]);
		i++;
	}
}

int drawPupShadow(POWERUP * p)
{
	if(p->active)
	{
		DDraw_Draw_Surface(pupShadowTextures[p->type],(int)p->loc.x+8,(int)p->loc.y+8,32,16,surface,1);
	}
	return 1;
}

int drawPup(POWERUP * p)
{
	if(p->active)
	{
		DDraw_Draw_Surface(pupTextures[p->type],(int)p->loc.x,(int)p->loc.y,32,16,surface,1);	
	}
	return 1;
}

void drawPups()
{
	int i=0;
	while(i<10)
	{
		drawPup(&pups[i]);
		i++;
	}
}

void drawPupShadows()
{
	int i=0;
	while(i<10)
	{
		drawPupShadow(&pups[i]);
		i++;
	}
}

int drawBricks()
{
	int i=0;
	int x = BOUND_LEFT;
	int y = BOUND_UP;
	int j=0;

	SDL_Surface* shadow;
	shadow=createShadow(brickTextures[0][0],16,16);

	while(i<BHEIGHT)
	{
		j=0;
		x=BOUND_LEFT;
		while(j<(BWIDTH-1))
		{
			drawBrickShadow(&bricks[i][j],x,y,shadow,surface);
			drawBrick(&bricks[i][j],x,y,surface);
			j++;
			x+=16;
		}
		drawBrick(&bricks[i][j],x,y,surface);
		y+=16;
		i++;
	}
        SDL_FreeSurface(shadow);
	return 1;
}

int drawLasers(LASER * l)
{
	if(l==NULL)
		return 0;
	drawLasers(l->next);
	DDraw_Draw_Surface(paddle.bullet,(int)l->loc.x,(int)l->loc.y,8,8,surface,1);
	return 1;
}

int drawPaddle(PADDLE * p)
{
	int i=1;
	char hud[25];

	sprintf(hud,"%d %d",p->lasers,p->lives);
	drawFont(surface, &font,hud,0,p->loc.y);
	drawLasers(p->laserHead);

	if(p->lasers)
	{	
		DDraw_Draw_Surface(p->textures[LEFT_LASER],(int)p->loc.x,(int)p->loc.y,16,32,surface,1);
	}
	else
		DDraw_Draw_Surface(p->textures[LEFT],(int)p->loc.x,(int)p->loc.y,16,32,surface,1);
	while(i<(p->width-1))
	{
		DDraw_Draw_Surface(p->textures[MIDDLE],(int)p->loc.x+16*i,(int)p->loc.y,16,32,surface,1);
		i++;
	}
	if(p->lasers)
		DDraw_Draw_Surface(p->textures[RIGHT_LASER],(int)p->loc.x+16*i,(int)p->loc.y,16,32,surface,1);
	else
		DDraw_Draw_Surface(p->textures[RIGHT],(int)p->loc.x+16*i,(int)p->loc.y,16,32,surface,1);
	return 1;
}

int Game_Intro()
{
	Game_Main_Func = Game_Main;
	return 1;
}


int Game_Paused()
{
	return 1;
}

int Level_Intro()
{	
	static int waitStart=30;
	static int velStart=-22;
	static int vel=velStart;
	static int acc=-1;
	static int x;
	static int y=480;
	static int wait=waitStart;

	timer.start();

	int len=getLength(&font,levelName);

	if(vel!=0)
		vel-=acc;
	if(vel==0)
	{
		wait--;
		y--;
		if(wait<=0)
		{
			wait=waitStart;
			acc=1;
			vel-=acc;
		}
	}
	y+=vel;
	if(y<=0)
	{
		y=480;
		vel=velStart;
		acc=-1;
		wait=waitStart;
		Game_Main_Func = Game_Main;
	}

        SDL_FillRect(surface, 0, 0);
	DDraw_Draw_Surface(background,96,32,448,416,surface,0);
//	drawScoreboard();
	drawBricks();
	drawBall(ballHead);
	drawPaddle(&paddle);
	drawPups();
	drawMessages(surface);
	drawConsole();
	drawFont(surface, &font,levelName,320-(len>>1),y);
	updateScreen();

	timer.wait(20);
	return 1;
}


int Level_Finished()
{	
	return 1;
}

int Game_Over()
{	
	/*
	int i=0;
	int j=0;
	static int count = 640;
	static int accel = 10;
	static int halflen = -1;
	static char message[50];
	timer.start();
	DDraw_Fill_Surface(surface,0);
	level->drawLevel(surface);
	
	if(halflen==-1)
	{
		sprintf(message,"GAME OVER"); 
	}

	while(i<strlen(message))
	{
		if(message[i]!=' ')
			DDraw_Draw_Surface(font[message[i]-65],count+28*i,210,32,32,surface,1);
		i++;
	}
	if(halflen == -1)
	{
		halflen = (28*i)/2;
	}
	count-=accel;
	if((count+halflen)<400 && (count+halflen)>340)
	{
		if(accel>1)
			accel--;
	}
	else if((count+halflen) >240 && (count+halflen) < 300)
	{
		if(accel<10)
			accel++;
	}
	if(count+28*i<0)
	{
		count=640;
		halflen=-1;
		levelNumber=0;
		scores[0]=0;
		lives[0]=2;
		fetchLevel();
		Game_Main_Func=Game_Intro;
	}
	while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));

	timer.wait(12);	
	return 1;
	*/
	return 1;
}

void drawScoreboard()
{
	int c=0;
	int i=0;
	int x=20;
	int y=10;
	int s=curBlockCount;
	char str[16];
	
	sprintf(str,"%d/%d",curBlockCount,bestBlockCount);
	drawFont(surface, &font,str,x,y);

	y+=font.point;
	sprintf(str,"%d",score);
	drawFont(surface, &font,str,x,y);
}

int AI_MovePaddle(PADDLE * p,BALL * b)
{
	BALL * closest=NULL;
	int closestTime=1000;
	while(b!=NULL)
	{
		if(b->dir.y>0)
		{
			int time=(p->loc.y-b->loc.y)/(b->speed*b->dir.y);
			
			if(time<closestTime)
			{
				closestTime=time;
				closest=b;
			}
		}
		b=b->next;
	}
	if(closest==NULL && ballHead!=NULL)
		closest=ballHead;
	if(closest!=NULL)
	{
		if(abs(closest->loc.x-(p->width<<3)-p->loc.x)<48)
			setPaddleX(p,closest->loc.x-(p->width<<3));
		else
		{
			if(closest->loc.x<p->loc.x)
				setPaddleX(p,p->loc.x-48);
			else setPaddleX(p,p->loc.x+48);
		}
	}
	
	return 1;
}

int Game_Main()
{	
	static int x =0;
	static int y =0;
	int i=64;
	int j=32;
	timer.start();

	
	SDL_FillRect(surface,0,0);
	DDraw_Draw_Surface(background,96,32,448,416,surface,0);
	drawBallShadow(ballHead);
	drawPupShadows();
	drawScoreboard();
	drawBricks();
	drawBall(ballHead);
	drawPaddle(&paddle);
	drawPups();
	drawMessages(surface);
	drawConsole();

	//AI_MovePaddle(&paddle,ballHead);
	//movePaddle(&paddle,keys);
	
	if(!paused)
	{

		//GetCursorPos(&mpos);
		setPaddleX(&paddle,(mpos.x-(paddle.width<<3)));
		movePaddleLasers(&paddle,bricks);
		moveBall(ballHead);
		hitTheBricks(bricks,ballHead);
		movePups();
	}
	else drawFont(surface, &font,"Paused",320-(getLength(&font,"Paused")>>1),240);
	checkForCollision(&paddle,ballHead);
	{
		i=0;
		while(i<10)
			checkForCollisionPup(&paddle,&pups[i++]);
	}

	if(keys[SDLK_SPACE])
	{
		BALL * b = ballHead;
		while(b)
		{
			if(b->stuckTo)
			{
				b->stuckTo=NULL;
				break;
			}
			b=b->next;
		}
		
		if(paddle.lasers)
		{
			int i=0;
			LASER * l1=NULL;
			LASER * l2=NULL;
			paddle.laserReloadedIn--;
			if(paddle.laserReloadedIn<=0)
			{
				paddle.laserReloadedIn=5;
				while(i<MAX_LASERS)
				{
					if(!lasers[i].active)
					{
						if(l1==NULL)
						{
							l1=&lasers[i];
						}
						else if(l2==NULL)
						{
							l2=&lasers[i];
							break;
						}
					}
					i++;
				}
				if(l1!=NULL&&l2!=NULL)
				{
					initLaser(l1);
					initLaser(l2);
					l1->active=true;
					l2->active=true;
					l1->next=l2;
					l2->next=NULL;
					if(paddle.laserHead==NULL)
					{
						paddle.laserHead=l1;
					}
					else
					{
						LASER * finger = paddle.laserHead;
						while(finger->next!=NULL)
						{
							finger = finger->next;
						}
						finger->next=l1;
					}
					l1->loc.x=paddle.loc.x+4;
					l2->loc.x=paddle.loc.x+paddle.width*16-12;
					l1->loc.y=l2->loc.y=paddle.loc.y;
					paddle.lasers--;
			
				}
			}
		}

	}
	
	updateScreen();
	
	timer.wait(20);
	if(slow)
		timer.wait(100);
	return 1;
}

#ifdef ENABLE_SOUND
int loadSounds()
{
	breakSounds[0]=FSOUND_Sample_Load(FSOUND_FREE,"./sounds/break0.wav",FSOUND_NORMAL,0);
	breakSounds[1]=FSOUND_Sample_Load(FSOUND_FREE,"./sounds/break1.wav",FSOUND_NORMAL,0);
	return 1;
}
int freeSounds()
{
	FSOUND_Sample_Free(breakSounds[0]);
	FSOUND_Sample_Free(breakSounds[1]);

	return 1;
}
int playBreakSound()
{
	channel+=1;
	channel%=8;
	FSOUND_PlaySound(channel,breakSounds[1]);return 1;
}
#endif

int exitFunc(char * str)
{
  exit(1);
  return 1;
}

int mapFunc(char * str)
{
	char * ptr = strchr(str,' ');
	readLevel(ptr+1,bricks);
	return 1;
}

int giveFunc(char * str)
{
	char * ptr = strchr(str,' ');
	char * lwr=ptr + 1; // strlwr(ptr+1);
	if(strstr(lwr,"sticky")==lwr)
	{
		int value=1;
		char tstr[12];

		sscanf(lwr,"%s %d",tstr,&value);
		paddle.sticky=value;
	}
	else if(strstr(lwr,"lasers")==lwr)
	{
		int bullets=20;
		char temp[25];
	
		paddle.lasers=bullets;
		
		if(sscanf(lwr,"%s %d",temp,&bullets))
			paddle.lasers=bullets;
		
	}
	else if(strstr(lwr,"width")==lwr)
	{
		int width=paddle.width;
		ptr = strchr(lwr,' ') + 1;
		if(sscanf(ptr,"%d",&width)==1)
			setPaddleWidth(&paddle,width);
	}
	else if(strstr(lwr,"powerball")==lwr)
	{
		BALL * bal=ballHead;
		if(bal)
		{
			bal->powerBall=true;
		}
	}
	else if(strstr(lwr,"ball")==lwr)
	{
		BALL * bal;
		bal=ballHead;
		if(ballHead==NULL)
		{
			ballHead=getBall();
			initBall(ballHead,0,0);
			stickBallPaddle(ballHead,&paddle);
			return 1;
		}
		while(bal->next)
		{
			bal = bal->next;
		}
		bal->next = getBall();
		initBall(bal->next,0,0);
		stickBallPaddle(bal->next,&paddle);
		
	}
	return 1;
}

int ballFunc(char * str)
{
	char * ptr = strchr(str,' ');
	char temp[25];
	ptr = ptr + 1 ; // strlwr(ptr+1);
	if(sscanf(ptr,"%f %f %f",&ballHead->dir.x,&ballHead->dir.y,&ballHead->speed)==3)
	{
	}
	//if(sscanf(ptr,"%s %f",temp,&ball.speed)==2)
	//	;
	

	normalize(&ballHead->dir);
	return 1;
}

int Game_Init()
{
	
  if(fullscreen)
  {
    assert(false);
  }
  else
  {
    int rendererFlags = SDL_RENDERER_SOFTWARE, windowFlags = 0;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
      printf("Couldn't initialize SDL: %s\n", SDL_GetError());
      exit(1);
    }

    window =
      SDL_CreateWindow("Shooter 01", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

    if (!window)
    {
      printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
      exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    renderer = SDL_CreateRenderer(window, -1, rendererFlags);

    if (!renderer)
    {
      printf("Failed to create renderer: %s\n", SDL_GetError());
      exit(1);
    }
  }

	
  background = SDL_CreateRGBSurface(0, 448,416, 32, 0,0,0,0);
  SDL_FillRect(background, nullptr, SDL_MapRGB(background->format, 0,102,102));
	
  test = SDL_CreateRGBSurface(0, 48,16, 32, 0, 0, 0, 0);
  createBrickTexture(test,BRICK_SOLO,50,10,100);


  test2 = SDL_CreateRGBSurface(0, 48,16, 32, 0, 0, 0, 0);
  SDL_FillRect(test2, nullptr, SDL_MapRGB(background->format, 100,40,20));

  ballTexture = SDL_LoadBMP("textures/ball.bmp");
  SDL_SetColorKey(ballTexture, 1, 0);
  SDL_SetColorKey(test, 1, 0);
  SDL_SetColorKey(test2, 1, 0);
    
  int i=0;
  while(i<4)
  {
    char fname[255];
    sprintf(fname,"textures/pball%d.bmp",i);
    pballTextures[i] = SDL_LoadBMP(fname);
    SDL_SetColorKey(pballTextures[i], 1, 0);
    i++;
  }

  ballShadowTexture=createShadow(ballTexture,16,16);

  char filenames[NUM_PUPS][32]={"sticky","width","lasers","ball","powerball","megaball","megalasers","warp","life","shrink",
                                "wonky","reverse","death"};
  i=0;
  while(i<NUM_PUPS)
  {
    char actualName[255];
    sprintf(actualName,"textures/pup%s.bmp",filenames[i]);
                
    pupTextures[i] = SDL_LoadBMP(actualName);
    pupShadowTextures[i]=createShadow(pupTextures[i],32,16);

    SDL_SetColorKey(pupTextures[i], 1, 0);
    SDL_SetColorKey(pupShadowTextures[i], 1, 0);
    i++;
  }

  memset(lasers,0,sizeof(LASER)*MAX_LASERS);
  initPaddle(&paddle);
  {
    SDL_Surface* l, *m, *r, *ll, *rl, *bull;


    l = SDL_LoadBMP("textures/paddleleft.bmp");
    m = SDL_LoadBMP("textures/paddlemiddle.bmp");
    r = SDL_LoadBMP("textures/paddleright.bmp");

    SDL_SetColorKey(l, 1, 0);
    SDL_SetColorKey(m, 1, 0);
    SDL_SetColorKey(r, 1, 0);

    ll = SDL_LoadBMP("textures/paddleleftlaser.bmp");
    rl = SDL_LoadBMP("textures/paddlerightlaser.bmp");
    bull = SDL_LoadBMP("textures/bullet.bmp");

    SDL_SetColorKey(ll, 1, 0);
    SDL_SetColorKey(rl, 1, 0);
    SDL_SetColorKey(bull, 1, 0);
    
    setPaddleTextures(&paddle,l,m,r);
    setPaddleLasers(&paddle,ll,rl,bull);
  }

  memset(balls,0,sizeof(BALL)*MAX_BALLS);
  
  initBall(&balls[0],paddle.loc.x,paddle.loc.y);
  normalize(&balls[0].dir);
  stickBallPaddle(&balls[0],&paddle);
	
  ballHead=&balls[0];
  ballHead->next=NULL;
	
  //paddle.sticky=1;

  {
    int i=0;
    while(i<MAX_PUPS)
    {
      initPup(&pups[i]);
      pups[i].type=i%NUM_PUPS;	
				 
      i++;		
    }
  }
	
  /*
    init brick textures
  */
  /*
    {
    int i=0;
    while(i<BRICK_NUM_TYPES)
    {
    brickTextures[0][i]=DDraw_Create_Surface(32,16,DDSCAPS_VIDEOMEMORY,0);
    createBrickTexture(brickTextures[0][i],i,50,10,100);		
    i++;
    }
    }
    {
    int i=0;
    int j=0;
    while(i<BHEIGHT)
    {
    j=0;
    while(j<BWIDTH)
    {
    bricks[i][j].health=1;
    bricks[i][j].type = j%2 + 1;
    bricks[i][j].texture=0;
    j++;
    }
    i++;
    }
    }*/

  initMessages();
  loadFont(&font,"fonts/font30_2",30);
  loadFont(&font12,"fonts/font",12);
  readLevel("long.txt",bricks);

  initConsole(20,&font12);
  addCommand("exit",exitFunc);
  addCommand("map",mapFunc);
  addCommand("give",giveFunc);
  addCommand("ball",ballFunc);
#ifdef ENABLE_SOUND
  FSOUND_Init(44100, 32, 0);
  loadSounds();
#endif
	
  return 0;
}

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
  int i=0;
  int j=0;
  if(background)
  {
    SDL_FreeSurface(background);
  }
  while(i<BRICK_NUM_COLORS)
  {
    j=0;
    while(j<BRICK_NUM_TYPES)
    {
      if(brickTextures[i][j])
        SDL_FreeSurface(brickTextures[i][j]);
      j++;
    }
    i++;
  }

  messagesShutdown();

  i=0;
  while(i<NUM_PUPS)
  {
    if(pupTextures[i])
    {
      SDL_FreeSurface(pupTextures[i]);
      pupTextures[i]=NULL;
    }
    if(pupShadowTextures[i])
      SDL_FreeSurface(pupShadowTextures[i]);
    i++;
  }
  if(test)
  {
    SDL_FreeSurface(test);
  }
  if(test2)
  {
    SDL_FreeSurface(test2);
  }
  shutdownConsole();

  SDL_FreeSurface(ballTexture);
  i=0;
  while(i<4)
  {
    if(pballTextures[i])
      SDL_FreeSurface(pballTextures[i]);
    i++;
  }
  if(ballShadowTexture)
    SDL_FreeSurface(ballShadowTexture);
  //freeBall(&ball);
#ifdef ENABLE_SOUND
  freeSounds();
  FSOUND_Close();
#endif
  return 1;
}

int HandleInput();

int main(int ac, char* av[]) {
  // initialize game here
  Game_Init();
  
  // enter main event loop
  Game_Main_Func = Level_Intro;
  while(true)
  {
    surface = SDL_GetWindowSurface(window);

    HandleInput();
    
    Game_Main_Func();

    SDL_UpdateWindowSurface(window);
  }
  
  Game_Shutdown();
  
  return 0;
}

int HandleKeyPress(SDL_Keysym keysym) {
  int wparam = keysym.sym;
  std::cout << "text input:" << wparam << "\n";
  const char* name = SDL_GetKeyName(keysym.sym);
  if (keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT)) {
    if (name && name[0] == ';')
      wparam = ':';
  }


  if(consoleFunc(wparam))
    return 1;
  {
    if(wparam=='p'||wparam=='P')
      paused=!paused;
    if(wparam=='s'||wparam=='S')
    {
      slow=!slow;
    }
    if(wparam==SDLK_ESCAPE)
    {
      if(Game_Main_Func==Game_Main)
      {
        Game_Main_Func = Game_Paused;
      }
      else
      {
        if(Game_Main_Func == Game_Paused)
          Game_Main_Func = Game_Main;
      }
    }
    return(0);
  }
  return 0;
}

int HandleInput() {
  int wparam = 0;
  SDL_Event event;
  while (SDL_PollEvent(&event)) 
    switch(event.type)
    {
    case SDL_MOUSEMOTION:
      mpos.x = event.motion.x;
      mpos.y = event.motion.y;
      break;
    case SDL_MOUSEBUTTONDOWN:
      break;
    case SDL_QUIT:
      exit(0);
      break;

    case SDL_KEYDOWN:
      {
        SDL_Keysym keysym = event.key.keysym; 
        std::cout << "Keyboard:" << (char)keysym.sym << "\n";
        if (!keys[keysym.sym]) {
          HandleKeyPress(keysym);
        }
        keys[keysym.sym] = true;            
      }
      break;
    case SDL_KEYUP:
      {
        SDL_Keysym keysym = event.key.keysym;
        keys[keysym.sym] = false;
      }
      break;
    }

  return 0;

}


