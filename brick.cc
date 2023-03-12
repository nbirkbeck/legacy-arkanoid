#include "brick.h"
#include "font.h"
#include "pup.h"

extern SDL_Surface* brickTextures[BRICK_NUM_COLORS][BRICK_NUM_TYPES];
extern int bestBlockCount;
extern int curBlockCount;
extern int score;
extern POWERUP pups[MAX_PUPS];
extern bool slow;
extern Font font12;

int playBreakSound();

int drawBrickShadow(BRICK* b, int x, int y, SDL_Surface* shadow,
                    SDL_Surface* lpddsback) {
  if (b->state == normal) {
    DDraw_Draw_Surface(shadow, x + 8, y + 8, 16, 16, lpddsback, 0);
  }
  return 1;
}

int drawBrick(BRICK* b, int x, int y, SDL_Surface* lpddsback) {
  if (b->state == normal) {
    DDraw_Draw_Surface(brickTextures[b->texture][b->type], x, y, 16, 16,
                       lpddsback, 1);
  } else if (b->state == weakening) {

    DDraw_Draw_Surface(brickTextures[WEAKENING_TEXTURE][b->type], x, y, 16, 16,
                       lpddsback, 1);
    b->timer -= 2;
    if (b->timer <= 0) {
      b->timer = 16;
      b->state = normal;
    }
  } else if (b->state == breaking) {
    int xsize, ysize;
    int xoff, yoff;
    if (b->type == BRICK_HOR || b->type == BRICK_LEND ||
        b->type == BRICK_REND) {
      xsize = 16;
      ysize = b->timer;
      xoff = 0;
      yoff = (16 - b->timer) / 2;
      if (b->type == BRICK_LEND)
        xoff = yoff;
      if (b->type == BRICK_REND)
        xsize = ysize;
    } else {
      ysize = 16;
      xsize = b->timer;
      yoff = 0;
      xoff = (16 - b->timer) / 2;
      if (b->type == BRICK_TEND)
        yoff = xoff;
      if (b->type == BRICK_BEND)
        ysize = xsize;
    }
    DDraw_DrawSized_Surface(brickTextures[WEAKENING_TEXTURE][b->type], x + xoff,
                            y + yoff, 16, 16, xsize, ysize, lpddsback, 1);
    b->timer--;
    if (b->timer <= 0)
      b->state = invalid;
  }
  return 1;
}

/*
        returns 1 iff the brick at i j was hit.
*/
int hitBrick(BRICK bricks[BHEIGHT][BWIDTH], int x, int y) {
  int i, j;
  if (x < BOUND_LEFT || x >= BOUND_RIGHT)
    return 0;
  if (y < BOUND_UP || y >= BOUND_DOWN)
    return 0;
  j = (x - BOUND_LEFT) >> 4;
  i = (y - BOUND_UP) >> 4;

  if (i >= BHEIGHT || i < 0)
    return 0;
  if (j >= BWIDTH || j < 0)
    return 0;
  if (bricks[i][j].health > 0) {
    playBreakSound();
    curBlockCount++;
    score += 100;

    while (bricks[i][j].type == BRICK_HOR || bricks[i][j].type == BRICK_REND) {
      j--;
    }
    while (bricks[i][j].type == BRICK_VER || bricks[i][j].type == BRICK_BEND) {
      i--;
    }

    bricks[i][j].health--;
    if (bricks[i][j].health <= 0) {
      bricks[i][j].state = breaking;
      if (bricks[i][j].powerup != -1) {
        pups[bricks[i][j].powerup].active = 1;
        pups[bricks[i][j].powerup].loc.x = (float)x;
        pups[bricks[i][j].powerup].loc.y = (float)y;
      }
    } else
      bricks[i][j].state = weakening;
    if (bricks[i][j].type == BRICK_LEND) {
      while (bricks[i][j].type != BRICK_REND) {
        j++;
        bricks[i][j].health--;
        if (bricks[i][j].health <= 0) {
          bricks[i][j].state = breaking;
          if (bricks[i][j].powerup != -1) {
            pups[bricks[i][j].powerup].active = 1;
            pups[bricks[i][j].powerup].loc.x = (float)x;
            pups[bricks[i][j].powerup].loc.y = (float)y;
          }
        } else
          bricks[i][j].state = weakening;
      }
    } else if (bricks[i][j].type == BRICK_REND) {
      while (bricks[i][j].type != BRICK_LEND) {
        j--;
        bricks[i][j].health--;
        if (bricks[i][j].health <= 0) {
          bricks[i][j].state = breaking;
          if (bricks[i][j].powerup != -1) {
            pups[bricks[i][j].powerup].active = 1;
            pups[bricks[i][j].powerup].loc.x = (float)x;
            pups[bricks[i][j].powerup].loc.y = (float)y;
          }
        } else
          bricks[i][j].state = weakening;
      }
    } else if (bricks[i][j].type == BRICK_TEND) {
      while (bricks[i][j].type != BRICK_BEND) {
        i++;
        bricks[i][j].health--;
        if (bricks[i][j].health <= 0) {
          bricks[i][j].state = breaking;
          if (bricks[i][j].powerup != -1) {
            pups[bricks[i][j].powerup].active = 1;
            pups[bricks[i][j].powerup].loc.x = (float)x;
            pups[bricks[i][j].powerup].loc.y = (float)y;
          }
        } else
          bricks[i][j].state = weakening;
      }
    } else if (bricks[i][j].type == BRICK_BEND) {
      while (bricks[i][j].type != BRICK_TEND) {
        i--;
        bricks[i][j].health--;
        if (bricks[i][j].health <= 0) {
          bricks[i][j].state = breaking;
          if (bricks[i][j].powerup != -1) {
            pups[bricks[i][j].powerup].active = 1;
            pups[bricks[i][j].powerup].loc.x = (float)x;
            pups[bricks[i][j].powerup].loc.y = (float)y;
          }
        } else
          bricks[i][j].state = weakening;
      }
    }

    return 1;
  }
  return 0;
}

/*
        Did ball hit the bricks
*/
int hitTheBricks(BRICK bricks[BHEIGHT][BWIDTH], BALL* ball) {
  if (ball == NULL)
    return 0;
  int dy = (ball->dir.y > 0) ? 1 : 0;
  int dx = (ball->dir.x > 0) ? 1 : 0;

  int locx = (int)ball->loc.x;
  int locy = (int)ball->loc.y;

  float diag = 0.7f;

  if (ball->next)
    hitTheBricks(bricks, ball->next);
  if (ball->powerBall) {
    hitBrick(bricks, locx + dx * BALL_DIAM, locy + BALL_RAD);
    hitBrick(bricks, locx + BALL_RAD, locy + dy * BALL_DIAM);
    hitBrick(bricks, (int)(locx + BALL_RAD + BALL_RAD * ball->dir.x),
             (int)(locy + BALL_RAD + BALL_RAD * ball->dir.y));
    return 1;
  }
  if (hitBrick(bricks, locx + dx * BALL_DIAM, locy + BALL_RAD)) {
    ball->dir.x *= -1;
#ifdef BDBG
    char* str = (char*)malloc(32);
    sprintf(str, "x->");
    addMessage(str, &font12, locx, locy, 100);
#endif
  } else if (hitBrick(bricks, locx + BALL_RAD, locy + dy * BALL_DIAM)) {
    ball->dir.y *= -1;
#ifdef BDBG
    char* str = (char*)malloc(32);
    sprintf(str, "y->");
    addMessage(str, &font12, locx, locy, 100);
#endif
  } else if (hitBrick(bricks, (int)(locx + BALL_RAD + BALL_RAD * ball->dir.x),
                      (int)(locy + BALL_RAD + BALL_RAD * ball->dir.y))) {
    // ball->speed=0;
    ball->dir.y *= -1;
    ball->dir.x *= -1;

#ifdef BDBG
    char* str = (char*)malloc(32);
    sprintf(str, "xy->");
    addMessage(str, &font12, locx, locy, 100);
#endif
  }

  /*
          New diagonal checks
  */
  else if (hitBrick(bricks, locx + 3, locy + 3)) {

    // ball->speed=0;
    if (!dx)
      ball->dir.x *= -1;
    ball->dir.y *= -1;
    // ball->dir.x*=-1;
#ifdef BDBG
    char* str = (char*)malloc(32);
    sprintf(str, "new1");
    addMessage(str, &font12, locx, locy, 100);
#endif

  } else if (hitBrick(bricks, locx + 12, locy + 3)) {
    // ball->speed=0;
    ball->dir.y *= -1;
    if (dx)
      ball->dir.x *= -1;
#ifdef BDBG
    char* str = (char*)malloc(32);
    sprintf(str, "new2");
    addMessage(str, &font12, locx, locy, 100);
#endif
  } else if (hitBrick(bricks, locx + 3, locy + 12)) {
    // ball->speed=0;
    if (dy)
      ball->dir.y *= -1;
    ball->dir.x *= -1;
#ifdef BDBG
    char* str = (char*)malloc(32);
    sprintf(str, "new3");
    addMessage(str, &font12, locx, locy, 100);
#endif
  } else if (hitBrick(bricks, locx + 12, locy + 12)) {
    // ball->speed=0;
    if (!dy)
      ball->dir.y *= -1;
    ball->dir.x *= -1;
#ifdef BDBG
    char* str = (char*)malloc(32);
    sprintf(str, "new4");
    addMessage(str, &font12, locx, locy, 100);
#endif
  }
  return 1;
}

int createBrickTexture(SDL_Surface* lpdds, int type, int r, int g,
                       int b) // transparency flag
{
  {
    SDL_Rect rect = {0, 0, 16, 16};
    SDL_FillRect(lpdds, &rect, SDL_MapRGB(lpdds->format, r, g, b));
  }

  // ddbltfx.dwFillColor =
  // _RGB16BIT565((r>=31)?r:r+1,(g>=31)?g:g+1,(b>=31)?b:b+1);
  int dwFillColor = SDL_MapRGB(
      lpdds->format, (int)((((float)r) * 0.7f) + ((float)(0.3f * 0xf1))),
      (int)((((float)g) * 0.7f) + ((float)(0.3f * 0xf1))),
      (int)((((float)b) * 0.7f) + ((float)(0.3f * 0xf1))));
  /*
          Doing the top part
  */
  if (type != BRICK_VER && type != BRICK_BEND) {
    SDL_Rect rect = {0, 0, 16, 1};
    SDL_FillRect(lpdds, &rect, dwFillColor);
  }

  if (type != BRICK_HOR && type != BRICK_REND) {
    SDL_Rect rect = {0, 0, 1, 16};
    SDL_FillRect(lpdds, &rect, dwFillColor);
  }
  // ddbltfx.dwFillColor =
  // _RGB16BIT565((r>=31)?r:r+1,(g>=31)?g:g+1,(b>=31)?b:b+1);
  dwFillColor = SDL_MapRGB(
      lpdds->format, (int)((((float)r) * 0.85f) + ((float)(0.15f * 0xf1))),
      (int)((((float)g) * 0.85f) + ((float)(0.15f * 0xf1))),
      (int)((((float)b) * 0.85f) + ((float)(0.15f * 0xf1))));
  /*
          Doing the top part
  */
  if (type != BRICK_VER && type != BRICK_BEND) {
    SDL_Rect rect = {1, 1, 15, 1};
    SDL_FillRect(lpdds, &rect, dwFillColor);
    /*
    rect.left    = 1;
    rect.top     = 1;
    rect.right   = 16;
    rect.bottom  = 2;
    */
  }

  if (type != BRICK_HOR && type != BRICK_REND) {
    SDL_Rect rect = {0, 0, 1, 16};
    SDL_FillRect(lpdds, &rect, dwFillColor);
    /*
    rect.left    = 0;
    rect.top     = 0;
    rect.right   = 1;
    rect.bottom  = 16;
    */
  }

  // ddbltfx.dwFillColor = _RGB16BIT565((r>=1)?r-1:r,g,(b>=1)?b-1:b);
  dwFillColor = SDL_MapRGB(lpdds->format, (int)(((float)r) * 0.8f),
                           (int)(((float)g) * 0.8f), (int)(((float)b) * 0.8f));

  if (type != BRICK_HOR && type != BRICK_LEND) {
    SDL_Rect rect = {14, 0, 2, 16};
    SDL_FillRect(lpdds, &rect, dwFillColor);
    /*
          rect.left    = 14;
          rect.top     = 0;
          rect.right   = 16;
          rect.bottom  = 16;
    */
  }

  if (type != BRICK_VER && type != BRICK_TEND) {
    SDL_Rect rect = {0, 14, 16, 2};
    SDL_FillRect(lpdds, &rect, dwFillColor);
    /*
          rect.left    = 0;
          rect.top     = 14;
          rect.right   = 16;
          rect.bottom  = 16;
    */
  }
  dwFillColor = SDL_MapRGB(lpdds->format, (int)(((float)r) * 0.6f),
                           (int)(((float)g) * 0.6f), (int)(((float)b) * 0.6f));

  if (type != BRICK_HOR && type != BRICK_LEND) {
    SDL_Rect rect = {15, 0, 1, 16};
    SDL_FillRect(lpdds, &rect, dwFillColor);
    /*
          rect.left    = 15;
          rect.top     = 0;
          rect.right   = 16;
          rect.bottom  = 16;
    */
  }

  if (type != BRICK_VER && type != BRICK_TEND) {
    SDL_Rect rect = {0, 15, 16, 1};
    SDL_FillRect(lpdds, &rect, dwFillColor);
    /*
          rect.left    = 0;
          rect.top     = 15;
          rect.right   = 16;
          rect.bottom  = 16;
    */
  }
  // return success
  return (1);
}
