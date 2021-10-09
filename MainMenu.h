#ifndef MAINMENU_H
#define MAINMENU_H

#include "engine/PrimitiveObject.h"

class GUI_Menu;
class Timer;
class Character;
class ScreenObject;
template <typename type>
class CombatGridLineTraceBehaviour;
class DemoScript;
class ParticleEmitter;
class Encounter;

class MainMenu : public PrimitiveObject {
public:
	MainMenu();

	virtual void destroy() override;

	void pressDemoButton();
	void pressRendererButton();

	void startDemo();
	void startRendererTest();

	void wakeUp();

	void hide();

	void fade();
	void fade2();
	void fade3();

	std::string smt() { return "etc"; }

private:
	virtual ~MainMenu();

	DemoScript* script;

	CombatGridLineTraceBehaviour <Character> *behaviour;
	GUI_Menu	*menu;
	Timer		*timer;
	Encounter	*encounter;

	ScreenObject* titlesCounter;
	std::vector <ScreenObject*> endingTitles;
	std::set <std::string> endingsSet;

	ScreenObject *title;
	GUI_Window *sh1, *sh1Hover, *sh2, *sh2Hover;

	bool bScroll, startingMotionFinished;

	virtual void loop(float delta) override;

	void addEnding(std::string ending);
	void playAnimation();
	void stopScroll();

	void spawnRandomEmitter();
	void spawnRainEmitter();
	void spawnRainEmitter2();
	void spawnSnowEmitter();
	void destroyEmitter();
	ParticleEmitter *emitter;

	friend class Debugger;
};

#endif	// MAINMENU_H
