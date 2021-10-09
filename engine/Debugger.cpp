#include "Debugger.h"

#include "Menu.h"
#include "Windows.h"
#include "Animator.h"
#include "AnimatorTable.h"
#include "ConsoleRenderer.h"
#include "Controller.h"
#include "ControllerManager.h"
#include "GodClass.h"
#include "AudioWorldEmitter.h"
#include "AudioManager.h"
#include "ScreenObjectTable.h"
#include "Timer.h"
#include "Particles.h"
#include "../Behaviours.h"
#include "../Encounter.h"
#include "../Narration.h"
#include "../Scripts.h"
#include "../MainMenu.h"
#include "../Player.h"
#include "../CharactersCore.h"

#define	DEBUG_STATS_MODE	true
#define	USE_LOG				true

std::list <std::pair <ScreenObject*, float>> Debugger::debugMsg = std::list <std::pair <ScreenObject*, float>>();
float Debugger::count = 0;
float Debugger::time = 0;

std::string strgen();

std::ofstream Debugger::debugLog = std::ofstream("C:\\Users\\quackdratic\\source\\repos\\ASCIIEngine\\engine\\log.txt");

Debugger::Debugger() : sum(0) {
	time = 0;
	count = life = 0;
	for (int i = 0; i < 60; ++i) deltas.push_back(0);
	debugStatsCtrl = nullptr;
}   

Debugger::~Debugger() { for (auto it : debugMsg) if (it.first) it.first->destroy(); }

void Debugger::loop(float delta) {
	life += delta;
	count += delta;
	if (count > time) stopDebugPause();

	auto it = debugMsg.begin();
	while (it != debugMsg.end())
		if (it->first) {
			if (it->first->lifespan > it->second) {
				auto  cpy = it->first;
				it = debugMsg.erase(it);
				cpy->destroy();
			}	else ++it;
		}	else break;

	int c = 0;
	for (auto it : debugMsg)
		if (it.first) it.first->setPosition(0, c++);
	deltas.push_back(delta);
	sum += delta;
	while (sum - deltas.front() > 1.0f)
		sum -= deltas.front(), deltas.pop_front();
}

void spawner() {
	DEBUG_PRINT("pressed!", 0.5f);
	for (int i = 0; i < 100; ++i) {
		int x = Rand() % 60;
		int y = Rand() % 60;
		new Animator("dummy 2", x, y, 100, true);
	}
}

void Debugger::initialize() {

	//	Controller* ctrl = new Controller(100);
	//	ctrl->setToggleEvent(KEY_PRESS, "Q", &spawner);
	//	ctrl->wakeUp();

	if (DEBUG_STATS_MODE) {
		GUI_WindowsTable* table = new GUI_WindowsTable("dummy", 60);
		win = new GUI_Window(1000, "dummy", getwidth() - 70, -1, true, 0);
		win->setName("debugWin");
		table->updateWindow(win, "first");
		win->setFunction("0", STRDISPLAY, &strgen);
		win->setFunction("1", STRDISPLAY, this, &Debugger::pobjCount);
		win->setFunction("7", STRDISPLAY, this, &Debugger::animCount);
		win->setFunction("3", STRDISPLAY, this, &Debugger::sobjCount);
		win->setFunction("4", STRDISPLAY, this, &Debugger::guiCount);
		win->setFunction("5", STRDISPLAY, this, &Debugger::GCtracked);
		win->setFunction("6", STRDISPLAY, this, &Debugger::getFPS);
		win->bLoopPostcedence = 1;
		win->color.setColor(GREEN, DARK);
		win->hide();

		debugStatsCtrl = new Controller(1000);
		debugStatsCtrl->setName("dbgCtrl");
		debugStatsCtrl->setToggleEvent(KEY_PRESS, "V", this, &Debugger::statsVisibilityToggle);
		debugStatsCtrl->wakeUp();
	}

	ScriptUtilities::player = new Player();
	ScriptUtilities::narrator = new Narration(PADDING, getheight() - 20, getwidth() - PADDING);
	ScriptUtilities::narrator->hideWindow();

	new MainMenu();
	GOD->getAudioManager()->playMusic("rain.mp3");
	return;

	return;

	std::pair <float, float> props2[EMITTER_PROPERTIES_COUNT] = {
		{1, 1.5},
		{0.5, 1},
		{0, 0},
		{0, 0},
		{0, 0},
		{-40, -20},
		{-50, 50},
		{30, 70}
	};

	std::vector <std::pair <float, std::string>> kineticMap;
	kineticMap.push_back({ 1.5, "|" });
	kineticMap.push_back({ 0.5, "." });
	kineticMap.push_back({ 1.0, ";" });
	new ParticleEmitter(100.f, { 100, 40 }, props2, new KineticFactory <100>(kineticMap));

	std::pair <float, float> props[EMITTER_PROPERTIES_COUNT] = {
		{1, 2},
		{1, 1},
		{0, 0},					// offset_x
		{0, 0},					// offset_y
		{50, 100},				// vel_x
		{-100, -50},			// vel_y
		{-400, -300},
		{400, 800}
	};

	auto bb = new ScreenObject(50, 50, 100, "");
	bb->updateShape(LineDraw::draw(0, 0, 20, 60, "/\\.."));

	new ParticleEmitter(1.f, { 100, 40 }, props, new ParticleFactory <FlashParticle, 100>);
}


void Debugger::startDebugPause(float time2) {
	count = 0; time = time2;
	clearscreen(); setcursor(0, 0);
	GOD->bPause = 1; 
	std::cout << GOD->aliveObjects.size() << '\n';
	if (USE_LOG) debugLog << GOD->aliveObjects.size() << '\n';
	for (auto it : GOD->aliveObjects) {
		std::cout << it->getIdentifierName() << ' ';
		if (USE_LOG) debugLog << it->getIdentifierName() << ' ';
	}
}
void Debugger::showDebugGamestate(std::string name) {
	std::cout << '\n' << " >>> " << name << ' ' << gamestates[name].size() << '\n';
	if (USE_LOG) debugLog << '\n' << " >>> " << name << ' ' << gamestates[name].size() << '\n';
	for (auto it : gamestates[name]) {
		std::cout << it->getIdentifierName() << ' ';
		if (USE_LOG) debugLog << it->getIdentifierName() << ' ';
	}
}

void Debugger::logText(const std::string& text) {
	if (USE_LOG) debugLog << text << '\n';
}

void Debugger::stopDebugPause() {
	GOD->bPause = 0;
}

void Debugger::statsVisibilityToggle() {
	if (!win->bHide) win->hide();
	else win->show();
}

void Debugger::saveGamestate(std::string name) {
	gamestates[name] = GOD->aliveObjects;
}

std::string strgen() {
	std::stringstream ss; ss << GOD->getGametime();
	return  std::string(ss.str());
}
std::string Debugger::getFPS() {
	return "FPS: " + intToStr((std::min)(120, (int)deltas.size()));
}
std::string Debugger::pobjCount() {
	return "alive obj: " + intToStr(GOD->aliveObjects.size());
}
std::string Debugger::animCount() {
	return "anim: " + intToStr(Animator::objCount);
}
std::string Debugger::sobjCount() {
	return "sobj: " + intToStr(ScreenObject::objCount);
}
std::string Debugger::guiCount() {
	return "win: " + intToStr(GUI_Window::objCount);
}
std::string Debugger::GCtracked() {
	return "other: " + intToStr(GOD->garbageTracked.size());
}