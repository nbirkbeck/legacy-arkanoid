#include "console.h"
#include "font.h"
#include <SDL2/SDL.h>

Command* commands;

extern SDL_Surface* surface;

SDL_Surface* consoleTexture = NULL;

int maxCommands = 0;
int numCommands = 0;
bool console = false;

char last[255];

char str[1024];
int stri = 0;
namespace {
Font* font;
}
/*
        void initConsole(int maxCommands)
        pre: nCommands >=0
        post: maxCommands are allocated and must be freed with a call
        to shutdownConsole when done
*/
void initConsole(int max, Font* pfont) {
  commands = (Command*)malloc(sizeof(Command) * max);
  maxCommands = max;
  numCommands = 0;
  font = pfont;
  sprintf(last, "");
  consoleTexture = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
  SDL_FillRect(consoleTexture, nullptr,
               0); // SDL_MapRGB(_RGB16BIT565(10,10,10));
}

int drawConsole() {
  int i = 0;
  static unsigned int blink = 0;
  int x, y;
  if (!console)
    return 0;

  // fade(0.5);
  while (i < 640) {
    SDL_Rect dest_rect = {i, 0, 32, 32};
    SDL_BlitSurface(consoleTexture, nullptr, surface, &dest_rect);
    i += 32;
  }
  x = 10;
  y = 12;
  drawFont(surface, font, ">", &x, &y);
  drawFont(surface, font, str, &x, &y);
  blink++;
  if ((blink >> 4) % 2)
    drawFont(surface, font, "_", &x, &y);

  return 1;
}
int playBreakSound();
int consoleFunc(int wparam) {
  if (!console) {
    if (wparam == '~' || wparam == '`') {
      console = true;
      return 1;
    } else
      return 0;
  }
  {
    if (wparam == SDLK_UP) {
      sprintf(str, "%s", last);
      stri = strlen(last);
      return 1;
    }
    if (wparam == SDLK_DOWN) {
      stri = 0;
      str[stri] = '\0';
      return 1;
    }
    if (wparam == '~' || wparam == '`') {

      console = false;
      return 1;
    }
    if (wparam >= 32 && wparam < 128) {
      str[stri++] = wparam;
      str[stri] = '\0';
      // keys[wparam]=false;
    } else if (wparam == SDLK_TAB) {
      int c = 0;
      char* start;

      if (str[0] == '/' || str[0] == '\\') {
        start = (str + 1);
      } else
        start = str;

      while (c < numCommands) {
        char* cptr = NULL;

        cptr = strstr(commands[c].name, start);
        if (cptr == commands[c].name) {
          sprintf(str, "\\%s ", commands[c].name);
          stri = strlen(str);
          //	playMenuItemSound();
          break;
        }

        cptr = strstr(start, commands[c].name);
        if (cptr == start) {
          char temp[255];
          sprintf(temp, "\\%s", start);
          sprintf(str, "%s", temp);
          stri = strlen(str);
        }
        c++;
      }
    } else if (wparam == SDLK_BACKSPACE) {
      stri--;
      if (stri < 0)
        stri = 0;
      str[stri] = '\0';
    } else if (wparam == SDLK_RETURN) {
      sprintf(last, "%s", str);
      if (str[0] == '/' || str[0] == '\\') {
        int c = 0;
        char command[255];
        sscanf(str + 1, "%s", command);
        while (c < numCommands) {

          if (strcmp(commands[c].name, command) == 0) {
            if (commands[c].func)
              commands[c].func(str + 1);
            break;
          }
          c++;
        }

        stri = 0;
        str[stri] = '\0';
        return 1;
      }

      stri = 0;
      str[stri] = '\0';
    }

    return 1;
  }
}

/*
        void addCommand
        pre: The name must be less than 24 chars, and the function must not be
   null post: The command is added to the list of commands in the consoles
        database
*/
void addCommand(const char* name, int (*func)(char*)) {
  if (numCommands < maxCommands) {
    commands[numCommands].func = func;
    sprintf(commands[numCommands].name, "%s", name);
    numCommands++;
  }
}

/*
        int shutdownConsole()
        pre: initConsole has been called
        post: the memory allocated is released
*/
int shutdownConsole() {
  if (commands)
    free(commands);
  if (consoleTexture)
    SDL_FreeSurface(consoleTexture);
  return 1;
}
