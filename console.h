#include "font.h"

typedef struct COMMAND_TYPE
{
	char name[32];
	int (* func)(char * );
}Command;


void initConsole(int nMessages,Font *);
int drawConsole();
int consoleFunc(int wparam);
void addCommand(const char * name,int (* func)(char *));
int shutdownConsole();
