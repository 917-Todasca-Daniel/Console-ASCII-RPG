#include "MainMenu.h"

#include "engine/Menu.h"
#include "engine/GodClass.h"
#include "engine/Timer.h"
#include "engine/Windows.h"
#include "engine/ConsoleRenderer.h"
#include "engine/ScreenObjectTable.h"
#include "engine/Debugger.h"
#include "engine/Particles.h"
#include "engine/AudioManager.h"

#include "Narration.h"
#include "Behaviours.h"
#include "Player.h"
#include "CharactersCore.h"
#include "Scripts.h"

#include "Encounter.h"


std::string display1() {
	return "PLAY DEMO";
}
std::string display1Hover() {
	return "P L A Y   D E M O";
}
std::string display2() {
	return "TEST COMBAT";
}
std::string display2Hover() {
	return "T E S T   C O M B A T";
}

MainMenu::MainMenu() {
	bScroll = true;
	emitter = nullptr;
	timer = nullptr;
	encounter = nullptr;

	menu = new GUI_Menu(std::vector <std::string> ({ "W", "UP" }), std::vector <std::string> ({ "S", "DOWN" }), 0, getheight() - 15, true);
	menu->bAutoDestroy = false;

	sh1 = new GUI_Window(50, "menuButton", 0, 0, false, 35);
	sh1->setFunction("0", STRDISPLAY, &display1, WHITE);
	
	sh1Hover = new GUI_Window(50, "menuButton", 0, 0, false, 35);
	sh1Hover->setFunction("0", STRDISPLAY, &display1Hover, YELLOW);
	
	sh2 = new GUI_Window(50, "menuButton", 0, 0, false, 35);
	sh2->setFunction("0", STRDISPLAY, &display2, WHITE);
	
	sh2Hover = new GUI_Window(50, "menuButton", 0, 0, false, 35);
	sh2Hover->setFunction("0", STRDISPLAY, &display2Hover, GREEN);

	menu->addButton(sh1, sh1Hover, getwidth()-50, 0, this, &MainMenu::pressDemoButton);
	menu->addButton(sh2, sh2Hover, getwidth()-50, 5, this, &MainMenu::pressRendererButton);

	ScreenObjectTable *scbt = new ScreenObjectTable("mainMenu");
	title = new ScreenObject(getwidth() - 65, getheight() - 21, 50, "");
	scbt->updateShape(title, "menuTitle");
	delete scbt;

	titlesCounter = new ScreenObject(5, getheight() - 9, 50, "");

	playAnimation();
}

void MainMenu::destroy() {	
	if (timer)		timer->destroy();
	if (menu)		menu->destroy();
	if (behaviour)	behaviour->destroy();
}

void MainMenu::wakeUp() {
	playAnimation();
	for (auto it : endingTitles)
		it->show();
	if (endingTitles.size() > 0) {
		// titlesCounter->show();
		// titlesCounter->updateShape(intToStr(endingTitles.size()) + "/3");
	}
	menu->show();
	menu->wakeUp();
	startingMotionFinished = false;
	bScroll = true;
}

void MainMenu::hide() {
	GOD->getRenderer()->resetBackground();
	PLAYERCH->getCharacter()->hide();
}

MainMenu::~MainMenu() { }

void MainMenu::loop(float delta) {
	PrimitiveObject::loop(delta);

	if (script && script->isOver()) {
		NARRATOR->freeze();
		addEnding(script->getEnding());
		new Timer(0.6f, script, &DemoScript::destroy);
		script->hideMap();
		script = nullptr;
		fade();
		new Timer(1.0f, this, &MainMenu::wakeUp);
	}

	if (encounter != nullptr) {
		return;
	}

	if (notifiers.find("movement behaviour finished") != notifiers.end()) {
		notifiers.erase("movement behaviour finished");
		GOD->getRenderer()->addScrollAmount(1.0f);
		startingMotionFinished = true;
	}

	if (startingMotionFinished && bScroll) GOD->getRenderer()->addScrollAmount(delta);
}

void MainMenu::addEnding(std::string ending) {
	if (endingsSet.find(ending) != endingsSet.end()) return;
	endingsSet.insert(ending);
	int size = endingTitles.size();
	endingTitles.push_back(new ScreenObject(5, getheight() - 8 + size, 50, ""));
	endingTitles.back()->updateShape(ending);
	endingTitles.back()->hide();
}

void MainMenu::playAnimation() {
	spawnRandomEmitter();
	notifiers.clear();
	for (auto it : PLAYERCH->getParty()) it->hide();
	
	title->show();
	menu->show();

	GOD->getRenderer()->resetBackground();
	GOD->getRenderer()->addBackground("forestbg2", true, 0.23f);
	GOD->getRenderer()->addBackground("forestbg1", true, 0.55f);
	GOD->getRenderer()->addBackground("forestbg3", true, 1.3f);
	
	PLAYERCH->getCharacter()->setPosition(0, 33);
	std::vector <std::pair <int, int>> arr = { {0, 33}, { 40, 33 } };
	behaviour = new CombatGridLineTraceBehaviour <Character>(arr, PLAYERCH->getCharacter(), 8);
	
	PLAYERCH->getCharacter()->show();
	PLAYERCH->getCharacter()->requestAnimation("walk_guitar", "walk_guitar");

	new ScrollAppearance <GUI_Window>(10.0f, sh1);
	new ScrollAppearance <GUI_Window>(10.0f, sh1Hover);
	new ScrollAppearance <GUI_Window>(10.0f, sh2);
	new ScrollAppearance <GUI_Window>(10.0f, sh2Hover);
}

void MainMenu::pressDemoButton() {
	PLAYERCH->tags.clear();
	menu->hide();
	menu->freeze();
	title->hide();

	NARRATOR->wakeUp();
	for (auto it : endingTitles) it->hide();
	titlesCounter->hide();

	stopScroll();
	timer = new Timer(2.0f, this, &MainMenu::startDemo);
	PLAYERCH->getCharacter()->requestAnimation("sheate", "idle");
	new Timer(1.0f, this, &MainMenu::fade);
}

void MainMenu::pressRendererButton() {
	menu->hide();
	menu->freeze();
	title->hide();

	for (auto it : endingTitles) it->hide();
	titlesCounter->hide();

	stopScroll();
	timer = new Timer(2.0f, this, &MainMenu::startRendererTest);
	PLAYERCH->getCharacter()->requestAnimation("sheate", "idle");
	new Timer(1.0f, this, &MainMenu::fade);
}

void MainMenu::stopScroll() {
	if (behaviour != nullptr && !behaviour->wasDestroyed()) behaviour->destroy();
	bScroll = false;
	PLAYERCH->getCharacter()->requestAnimation("idle_guitar");
}

void MainMenu::fade() {
	GOD->getRenderer()->overrideColor(DWHITE);
	new Timer(0.3f, this, &MainMenu::fade2);
	new Timer(0.6f, this, &MainMenu::fade3);
}
void MainMenu::fade2() {
	GOD->getRenderer()->overrideColor(GRAY);
}
void MainMenu::fade3() {
	hide();
	destroyEmitter();
	GOD->getRenderer()->resetOverride();
}

void MainMenu::startDemo() {
	script = new DemoScript();
}

void MainMenu::startRendererTest() {
	std::vector <Character*> characters;
	ScriptUtilities::player->getCharacter()->show();
	characters.push_back(ScriptUtilities::player->getCharacter());
	characters.push_back(new GolemCat("Golem Cat"));
	characters.push_back(new Archer("The Queen"));
	characters.push_back(new TrainingDummy("T. Dummy"));
	characters.push_back(new CHPaladin());
	encounter = new TrainingFight(characters);
	GOD->getAudioManager()->playMusic("lifewillchange.mp3");
	return;
}

void MainMenu::spawnRainEmitter() {
	std::pair <float, float> props2[EMITTER_PROPERTIES_COUNT] = {
		{1.5, 1.5},
		{1, 1},
		{0, getwidth()},
		{-4, -4},
		{0, 0},
		{20, 50},
		{0, 00},
		{10, 20}
	};

	std::vector <std::pair <float, std::string>> kineticMap;
	kineticMap.push_back({ 10, "|" });
	kineticMap.push_back({ 0.5, "'" });
	kineticMap.push_back({ 0.1, "x" });
	emitter = new ParticleEmitter(20.f, { 0, 0 }, props2, new KineticFactory <100>(kineticMap));
}

void MainMenu::spawnRainEmitter2() {
	std::pair <float, float> props2[EMITTER_PROPERTIES_COUNT] = {
		{1.5, 1.5},
		{1.2, 1.2},
		{-30, getwidth()-10},
		{-4, -4},
		{40, 40},
		{20, 50},
		{0, 00},
		{10, 20}
	};

	std::vector <std::pair <float, std::string>> kineticMap;
	kineticMap.push_back({ 10, "\\" });
	kineticMap.push_back({ 0.5, "'" });
	kineticMap.push_back({ 0.1, "x" });
	emitter = new ParticleEmitter(20.f, { 0, 0 }, props2, new KineticFactory <100>(kineticMap));
}

void MainMenu::spawnSnowEmitter() {
	std::pair <float, float> props2[EMITTER_PROPERTIES_COUNT] = {
		{1, 1.5},
		{1, 1},
		{0, getwidth()},
		{-4, -4},
		{-10, 10},
		{40, 50},
		{-30, -18},
		{-10, -12}
	};

	std::vector <std::pair <float, std::string>> kineticMap;
	kineticMap.push_back({ 10, "O" });
	kineticMap.push_back({ 1.0, std::string(1, char(248)) });
	kineticMap.push_back({ 0.1, "." });
	emitter = new ParticleEmitter(40.f, { 0, 0 }, props2, new KineticFactory <100>(kineticMap));
}

void MainMenu::spawnRandomEmitter() {
	if (emitter) return;
	int rand = randBetween(0.0, 2.0);
	if (rand == 0) spawnRainEmitter();
	else if (rand == 1) spawnRainEmitter2();
}

void MainMenu::destroyEmitter() {
	if (emitter) emitter->destroy();
	emitter = nullptr;
}
