#ifndef CONSOLEWIZARD_H_INCLUDED
#define CONSOLEWIZARD_H_INCLUDED

#define PADDING 2

#define _WIN32_WINNT 0x0500

#include <windows.h>

#include "stdc++.h"

enum COLOR : int;

HANDLE getHandle();

int  suspect_setcursor(short int x, short int y);
void setcursor(short int x, short int y);
void resizeconsole(short int width, short int height);
void forcesetcursor(short int x, short int y);

void chprint(int y, int x1, int x2, char ch);

void hidescroll();
void hidecursor();
void showcursor(short int thicc = 1);

void clearscreen();
void fullscreen();
void initialize();

void ctransform(int& x, int& y);
bool cvalid(int x, int y);

int getwidth();
int getheight();

#define ws wizard_singleton::getInstance()

class wizard_singleton {
private:
	wizard_singleton();
public:
	void updateclear();
	void update();
	static wizard_singleton* getInstance() {
		if (!instance) instance = new wizard_singleton;
		return instance;
	}   static wizard_singleton* instance;
	short int height, width;
	std::string clearstring;

	CONSOLE_CURSOR_INFO cursor;
	CONSOLE_SCREEN_BUFFER_INFO buffer;
	HANDLE console;
	COORD  pos;
};

#endif // CONSOLEWIZARD_H_INCLUDED
