#ifndef DEBUGGER_H
#define DEBUGGER_H

#define DEBUG_MODE true

#include "ScreenObject.h"

#include "consolewizard.h"

#include <iostream>
#include <string>
#include <list>

#define DEBUG_PRINT Debugger::print

class Debugger
{
public:
	Debugger();
	virtual ~Debugger();

	void initialize();
	void loop(float delta);

	template <typename type>
	static void print(int x, int y, type X, bool bPause = 0);

	template <typename type>
	static void print(type X, float lifespan);

	static void startDebugPause(float time);
	static void stopDebugPause();

	void statsVisibilityToggle();

	void saveGamestate(std::string name);
	void showDebugGamestate(std::string name);

	static void logText(const std::string &text);

protected:
	static std::ofstream debugLog;

	Controller *debugStatsCtrl;
	GUI_Window *win;

	static float count, time;
	float life;

	std::string pobjCount(), sobjCount(), animCount(), guiCount();
	std::string GCtracked();
	std::string getFPS();

	float sum;
	std::list <float> deltas;

	std::map <std::string, std::list <PrimitiveObject*>> gamestates;

	static std::list <std::pair <class ScreenObject*, float>> debugMsg;
};

template <typename type>
void Debugger::print(int x, int y, type X, bool bPause) {
	if (!DEBUG_MODE) return;
	setcursor(x, y); std::cout << X;
	if (bPause) std::cin.get();
}

template <typename type>
void Debugger::print(type X, float lifespan) {
	if (!DEBUG_MODE) return;
	debugMsg.push_back({ new ScreenObject(0, debugMsg.size(), 100000), lifespan });
	debugMsg.back().first->updateShape(X);
	debugMsg.back().first->setName("DBG");
}

#endif // DEBUGGER_H
