#include "consolewizard.h"

#include "Colors.h"

#include <iostream>
#include <sstream>

wizard_singleton* wizard_singleton::instance = 0;

wizard_singleton::wizard_singleton() {
	console = GetStdHandle(STD_OUTPUT_HANDLE);
}
void wizard_singleton::update() {
	GetConsoleScreenBufferInfo(console, &buffer);
	width = 62;
	height = buffer.dwSize.X;
	updateclear();
}
void wizard_singleton::updateclear() {
	int w = getwidth();
	int h = getheight();
	int len = w * h;
	clearstring = "";
	for (int i = 0; i < len; ++i)
		clearstring += " ";
}

HANDLE getHandle() { return ws->console; }

int suspect_setcursor(short int x, short int y) {
	x += PADDING;
	if (x < PADDING) {
		if (y < 0) return -1;
		else if (y >= getheight()) return -2;
		else return -3;
	}
	else if (x >= getwidth()) {
		if (y < 0) return 1;
		else if (y >= getheight()) return 2;
		else return 3;
	}
	if (y < 0) return 11;
	else if (y >= getheight()) return 12;
	SetConsoleCursorPosition(ws->console, ws->pos = { x, y });
	return 0;
}
void setcursor(short int x, short int y) {
	x += PADDING;
	SetConsoleCursorPosition(ws->console, ws->pos = { x, y });
}
void forcesetcursor(short int x, short int y) {
	SetConsoleCursorPosition(ws->console, ws->pos = { x, y });
}
void resizeconsole(short int width, short int height) {
	SetConsoleScreenBufferSize(ws->console, { ws->width = width, ws->height = height });
	SMALL_RECT shape = { (short)0, (short)0, (short)(ws->width - 1), (short)(ws->height - 1) };
	SetConsoleWindowInfo(ws->console, TRUE, &shape);
	ws->updateclear();
}

void hidescroll() {
	GetConsoleScreenBufferInfo(ws->console, &ws->buffer);
	ws->buffer.dwSize.X = ws->buffer.dwMaximumWindowSize.X;
	ws->buffer.dwSize.Y = ws->buffer.dwMaximumWindowSize.Y;
	SetConsoleScreenBufferSize(ws->console, ws->buffer.dwSize);
	ShowScrollBar(GetConsoleWindow(), SB_BOTH, FALSE);
}
void hidecursor() {
	ws->cursor.bVisible = 0;
	ws->cursor.dwSize = 1;
	SetConsoleCursorInfo(ws->console, &ws->cursor);
}
void showcursor(short int thicc) {
	ws->cursor.bVisible = 1;
	ws->cursor.dwSize = thicc;
	SetConsoleCursorInfo(ws->console, &ws->cursor);
}

void clearscreen() {
	setcursor(0, 0); std::cout << ws->clearstring;
}
void fullscreen() {
	ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
	ws->update();
}
void initialize() {
	fullscreen();
	hidescroll();
	hidecursor();
}

int getwidth()  { return ws->height; }
int getheight() { return ws->width; }

bool cvalid(int x, int y) {
	return (y >= 0 && y < getheight() && x >= PADDING && x < getwidth());
}

void ctransform(int& x, int& y) {
	x += PADDING;
}

void chprint(int y, int x1, int x2, char ch) {
	if (y >= 0 && y < getheight()) {
		int start = x1;
		if (x1 < 0)
			start = -x1, x1 = 0;
		if (x2 >= getwidth() - PADDING - 1)
			x2 = getwidth() - PADDING - 1;
		setcursor(x1, y);
		for (int i = start; i < x2; ++i)
			std::cout << ch;
	}
}