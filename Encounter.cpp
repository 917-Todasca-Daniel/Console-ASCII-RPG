#include "Encounter.h"

#include "engine/Menu.h"
#include "engine/GodClass.h"
#include "Behaviours.h"
#include "engine/Controller.h"
#include "engine/Windows.h"
#include "engine/ScreenObject.h"
#include "engine/Timer.h"
#include "engine/Animator.h"
#include "Narration.h"

#include "CharactersCore.h"

#include <queue>

#include <algorithm>
#include <fstream>

GridLayoutManager* GridLayoutManager::instance = 0;
Encounter* Encounter::instance = nullptr;
int GlobalTurnLoop::counter = 1;

std::string selectMove() { return "Select a move (W/S):"; }
std::string selectMove2() { return "Select a tile (W/A/S/D):"; }
std::string selectTarget() { return "Select a target (W/S):"; }
std::string selectTargets() { return "Select targets (W/S):"; }

std::string noTargetsInAttackRange() { return "No targets in range (ESCAPE to continue);"; }
std::string noTargetsInRange() { return "No targets in range (ENTER to continue)"; }

std::string getEndExplanation() { return "(press ENTER to continue)"; }

std::string attackBanner() { return "1.   Attacks"; }
std::string attackBannerOnHover() { return "1. A T T A C K S"; }
std::string spellBanner() { return "2.   Skills"; }
std::string spellBannerOnHover() { return "2. S K I L L S"; }
std::string moveBanner() { return "3.   Movement"; }
std::string moveBannerOnHover() { return "3. M O V E M E N T "; }
std::string commandBanner() { return "4.   End Turn"; }
std::string commandBannerOnHover() { return "4. E N D   T U R N "; }

int x = 2, y = -2, attack_off = 85;
int dummyx = 42, dummyy = 30;
int liney = 15;
int popup_width = 85;

Encounter::individualUIElements::individualUIElements() {
	for (int i = 0; i < UI_COUNT; ++i) elem[i] = nullptr;
}

Encounter::Encounter(std::string grid_name, std::vector <Character*> characters) {
	setName("Encounter");

	int objCount = 0;
	this->characters = characters;
	for (auto ch : characters) {
		profiles.push_back(ch->cp);
		if (ch->cp->sentience == OBJECT)
			ch->cp->name = "Object #" + intToStr(++objCount);
	}

	Character::resetTrackers();

	for (auto it : profiles) {
		it->init = new CombatProfile(*it);
		it->MP = it->startMP;
		it->AP = it->startAP;
		it->bFirstTurn = true;
	}

	for (int i = 0; i < (int)profiles.size(); ++i) {
		int x = (rand() % 2) * 2 - 1;
		int y = 0;
		profiles[i]->setVulnerableTile(POINT(x, y));
	}

	GlobalTurnLoop::counter = 1;

	pidx = (int)profiles.size() - 1;
	bAllowAI = false;
	bSimMode = false;
	bStandby = bTurnEnded = true;

	instance = this;
	commandPrompt = nullptr;

	LAYOUTS->initializeGrid(grid, grid_name);
	Animator::resetGlobalAnimSpeed();

	int min_x = 0;
	float delay = 0;
	std::map <int, float> lineDelay;
	for (auto tile : grid.tiles) {
		int line = tile->getPosition().second;
		if (lineDelay.find(line) == lineDelay.end()) lineDelay[line] = delay += 0.05f;
		new LeftToRightScan(min_x, 200, lineDelay[line], tile);
	}

	beginTimer = new Timer(1.5f, this, &Encounter::showScenario);
}

void Encounter::showScenario() {
	begin = new Narration(getwidth() / 2 - 35, getheight() / 2 - 14, 80, false, "narratorbox3");
	begin->playText("ENCOUNTER INFO:\n\nWin  Condition:    use Training Dummy's 'Win the Game' skill\nLose Condition:    use Training Dummy's 'Lose the Game' skill\n\n\nHints:\n    This is a training encounter meant for testing player characters' mechanics with no enemy AI.\n    AP - resource used to attack and skills.\n    MP - resource used for movement and skills. 1 tile = 1MP (no diagonals). Can move as many times as you want in a turn.\n    HP - health points, character dies when they drop below 0.\n    DR - damage reduction.\n    Attacks - standard way to deal damage, and most of the times the best! No cooldown.\n    Skills - most skills have cooldowns and are not meant only for damage.\n    Every characters takes a turn in the order displayed at the bottom of the window. At the start of their turn, they regen some AP and MP, but not HP.");

	speedSelect = new ScreenObject(getwidth() - 50, getheight() - 2, 100);
	speedSelect->updateShape("(press Z) MOVE SPEED:");
	std::string levels[] = { "NORMAL", "HIGH", "REALLY FAST" };
	int len = 0;
	for (int i = 0; i < 3; ++i) {
		speedLevels[i] = new ScreenObject(getwidth() - 28 + len, getheight() - 2, 100);
		speedLevels[i]->updateShape(levels[i]);
		speedLevels[i]->color = GRAY;
		len += levels[i].size() + 1;
		if (i == 0) speedLevels[i]->color = YELLOW;
	}
	speedSelect->hide();
	for (int i = 0; i < 3; ++i) speedLevels[i]->hide();
}

void Encounter::destroy() {
	PrimitiveObject::destroy();

	speedSelect->destroy();
	for (int i = 0; i < 3; ++i) {
		if (speedLevels[i])
			speedLevels[i]->destroy();
	}

	if (arrow) arrow->destroy();
	if (anim_hover_arrow) anim_hover_arrow->destroy();
	if (hover_arrow) hover_arrow->destroy();

	for (auto it : UIObj) if (it) {
		for (int i = 0; i < UI_COUNT; ++i) if (it->elem[i])
			it->elem[i]->destroy();
	}

	if (menus[ACT])   menus[ACT]->destroy();
	for (int i = 0; i < MENUTYPECOUNT; ++i)
		if (borders[i]) if (i != ACT) borders[i]->destroy();
	if (descriptions) descriptions->destroy();

	if (borders[ACT]) new FadeBehaviour <GUI_Window>(0.0f, 2.0f, borders[ACT], -1);

	LAYOUTS->gridClear(grid);
	handleErrorMessages(1000);

	for (auto it : characters) it->resetPriority();

	if (eventLogger) eventLogger->destroy();

	for (auto it : controllers) {
		if (it) it->destroy();
	}
}

Encounter::~Encounter() {
	for (auto it : rawHistory) {
		delete it.type;
		delete it.event;
	}

	for (auto it : triggers) {
		delete it;
	}
	if (brain) delete brain;
}

void Encounter::advanceAnimSpeedScale() {
	speedLevels[animSpeedScale]->color = GRAY;
	animSpeedScale = (animSpeedScale + 1) % 3;
	speedLevels[animSpeedScale]->color = YELLOW;
	//	Animator::setGlobalAnimSpeed(animSpeed[animSpeedScale]);
}

void Encounter::loop(float delta) {
	PrimitiveObject::loop(delta);

	for (int i = 0; i < characters.size(); ++i) 
		characters[i]->setColor(GRAY);

	manageInputControl();
	manageEndscreen();

	if (bFreezeFight) {
		handleErrorMessages(delta);
		return;
	}

	manageRenderPriorities();
	manageVulnerabilitiesRendering();

	if (bPrompted) {
		if (checkEscapePress()) { commandPrompt = nullptr; wakeUpControllers(); }
		else { runPromptTasks(); return; }
	}

	if (hover_arrow) hover_arrow->hide();
	if (anim_hover_arrow && !bInSelection) anim_hover_arrow->pause(), anim_hover_arrow->hide();

	runStack();
	handleEventLogger(delta);
	handleErrorMessages(delta);
	handleCharactersAnimations(delta);

	if (beginTimer != nullptr && !beginTimer->wasDestroyed()) return;
	currentProfile()->ch->setColor(WHITE);
	centerPopUp();
	updateUI(delta);

	if (currentProfile()->sentience == AI) {
		if (borders[POPUP]) borders[POPUP]->hide();
		if (borders[SPOPUP]) borders[SPOPUP]->hide();
	}

	if (bInMovement) {
		if (borders[POPUP])  borders[POPUP]->hide();
		if (borders[SPOPUP]) borders[SPOPUP]->hide();
		if (checkNotifier("movement behaviour finished")) {
			removeNotifier("movement behaviour finished");
			if (profiles[pidx]->sentience == AI) {
				pauseAI();
				new Timer(0.3f, this, &Encounter::restartAI);
			}
			bInMovement = false;
			LAYOUTS->clearAnim(grid);
		}
		else return;
	}

	if (bTilesSelection) {
		if (!checkEscapePress()) runTileSelection();
		return;
	}
	else {
		if (borders[SPOPUP]) borders[SPOPUP]->hide();
		if (movementCtrl) movementCtrl->freeze();
	}

	for (auto it : printRequests) eventLogger->addText(it.first, it.second);
	printRequests.clear();

	if (bTurnEnded) { runEndTurnTasks(); return; }

	if (begin) {
		if (begin->bEntered) {
			eventLogger->show();
			for (auto ctrl : controllers) ctrl->wakeUp();
			for (auto it : menus) if (it) it->wakeUp();
			speedSelect->show();
			for (int i = 0; i < 3; ++i) speedLevels[i]->show();
			begin->destroy();
			begin = nullptr;
		}
		else {
			eventLogger->hide();
			speedSelect->hide();
			for (int i = 0; i < 3; ++i) speedLevels[i]->hide();
			for (auto ctrl : controllers) ctrl->freeze();
			for (auto it : menus) if (it) it->freeze();
			return;
		}
	}

	if (!bAttackTargetRequest) descriptions->hide(), currentMoveOption = nullptr;
	if (bInSelection) { runSelectionTasks(); return; }
	else {
		endTurnCtrl->wakeUp();
		LAYOUTS->gridColor(grid, WHITE);
		if (menus[ACT] && !bTilesSelection) menus[ACT]->wakeUp();
	}

	checkEscapePress();

	if (currentProfile()->sentience == AI && bAllowAI) runAITasks();
}

void Encounter::killProfile(CombatProfile* profile) {
	profile->bDead = true;
	profile->name += " (dead)";
	profile->ch->requestAnimation("death", "death");
	if (profile->name == profiles[pidx]->name) endTurnPress();
}

void Encounter::setupPlayerTurn() {
	if (menus[ACT]) {
		if (!currentProfile()->getSpells().empty()) menus[ACT]->buttons[1].hoveredShape->setComponentColor(0, Color(CYAN));
		else menus[ACT]->buttons[1].hoveredShape->setComponentColor(0, Color(GRAY));
	}
	createMenus();
	Event* event = new Event();
	event->src = currentProfile();
	updateHistory <TurnBeginLog>(event);
	bStandby = bTurnEnded = false;
	currentProfile()->ch->speak(Character::SPEAKTYPES::TURNBEGIN);
	currentProfile()->turnBeginEffects();
}
void Encounter::endPlayerTurn() {
	menus[ACT]->freeze();
	bStandby = bTurnEnded = true;
}

void Encounter::runStack() {
	if (eventStack.empty()) return;
	auto& event = eventStack.back();
	eventStack.pop_back();
	if (event->bPositionChange) {
		if (event->src->sentience == PLAYER);
		else if (event->src)  LAYOUTS->paintTile(grid, event->src->getVulnerableTile(), 0);
		if (event->dest) LAYOUTS->paintTile(grid, event->dest->getVulnerableTile(), 0);
	}
	event->call();
}

void Encounter::handleEventLogger(float delta) {
	if (((bUp | bDown | bLeft | bRight) == false) || eventLogger->bHidden) pressTime = 0, log_x = floor(log_x), log_y = floor(log_y);
	else {
		float quantity = delta * 5.0f;
		float multiplier = 0.5f;
		if (pressTime == 0.0f) {
			if (!bUp && !bLeft) quantity += 1.0f, multiplier = 1.0f;
		}

		if (pressTime > 1.0f) quantity *= 15.0f;
		pressTime += delta;
		log_x += bRight * quantity - bLeft * quantity;
		log_y += bDown * quantity * multiplier - bUp * quantity * multiplier;
		log_x = min(getwidth() - 1, log_x); log_x = max(0, log_x);
		log_y = min(getheight() - 1, log_y);

		if (log_y < 8.0f) eventLogger->takeHit();
		log_y = max(8.0f, log_y);

		eventLogger->setPosition((int)log_x, (int)log_y);
	}
}

void Encounter::manageInputControl() {
	if (currentProfile()->sentience == AI) {
		if (singleSelector) singleSelector->menu->hide();
		if (multipleSelector && multipleSelector->selector()) multipleSelector->selector()->menu->hide();
		GOD->getControllerManager()->disableControllers();
		if (menus[ACT]) menus[ACT]->hide();
	}
	else {
		GOD->getControllerManager()->enableControllers();
		if (menus[ACT]) menus[ACT]->show();
	}
}

void Encounter::manageEndscreen() {
	if (bDestroyed) return;
	if (endMessageWin && !endMessageWin->bPlay) {
		endMessageWin->destroy();
		destroy();
	}

	if (!bFreezeFight) checkEndConditions();
}

void Encounter::manageRenderPriorities() {
	for (auto& it : characters) {
		if (it->cp->name == currentProfile()->name) it->resetPriority();
		else it->resetPriority();
	}
}

void Encounter::manageVulnerabilitiesRendering() {
	for (auto& it : profiles) {
		if (it->sentience == PLAYER && !bPlayerVulnerability) continue;
		if (bTilesSelection || bInMovement) continue;
		if (it == currentProfile() && bInSelection) continue;
		LAYOUTS->paintTile(grid, it->getVulnerableTile(), 6);
	}
}

void Encounter::runEndTurnTasks() {
	if (rawHistory.size() > 1) {
		Event* event = new Event();
		event->src = currentProfile();
		updateHistory <TurnEndLog>(event);
	}
	do {
		pidx = (++pidx == profiles.size() ? 0 : pidx);
	}	while (profiles[pidx]->sentience == OBJECT || profiles[pidx]->bDead);
	if (pidx == 0 && rawHistory.size() > 1) updateHistory<GlobalTurnLoop>(new GlobalTurnEvent());

	setupPlayerTurn();
	checkEscapePress();

	if (profiles[pidx]->sentience == AI) {
		bPrintExtendedStats = false;
		pauseAI();
		new Timer(0.5f, this, &Encounter::restartAI);
	}
}

void Encounter::runSelectionTasks() {
	if (currentProfile()->sentience == AI) {
		if (multipleSelector != nullptr) {
			if (multipleSelector->selector() != nullptr) {
				solveSelector(multipleSelector->selector());
			}
		}
	}

	if (endTurnCtrl) {
		if (bInSelection) endTurnCtrl->freeze();
		else			  endTurnCtrl->wakeUp();
	}

	if (currentProfile()->sentience != AI)
		borders[POPUP]->show();
	runSelections();
	checkEscapePress();
	if (bAttackTargetRequest) {
		int x = currentProfile()->getTile().first;
		int y = currentProfile()->getTile().second;
		LAYOUTS->gridColor(grid, currentProfile()->getAttacks()[0]->rangeTiles(x, y), currentProfile()->getAttacks()[0]->getColor());
	}
}

void Encounter::handleErrorMessages(float delta) {
	int error_lifetime = 5;
	int error_width = 50;
	if (!errorMessages.empty()) {
		for (auto it : errorMessages) {
			ScreenObject* obj = new ScreenObject(0, 0, 1000, "");
			obj->setName("error");
			obj->updateShape(it);
			obj->color = RED;
			errorMessagesObj.push_back({ obj, .0f });
		}	errorMessages.clear();
	}

	auto obj = errorMessagesObj.begin();
	for (int i = 0; i < (int)errorMessagesObj.size(); ++i) {
		obj->second += delta;
		if (obj->second > error_lifetime) {
			auto cpy = obj->first;
			obj = errorMessagesObj.erase(obj);
			--i;
			cpy->destroy();
		}
		else obj->first->setPosition(getwidth() - obj->first->estimateWidth() - 2, getheight() - (i + 1)), ++obj;
	}
}

void Encounter::handleCharactersAnimations(float delta) {
	for (auto it : profiles) it->pres.bMoving = false;
	if (bInMovement) currentProfile()->pres.bMoving = true;
	else {
		int x, y; x = y = 0;
		for (int i = 0; i < (int)characters.size(); ++i) {
			x = grid.pivot_x[profiles[i]->grid_y][profiles[i]->grid_x];
			y = grid.pivot_y[profiles[i]->grid_y][profiles[i]->grid_x];
			characters[i]->setPosition(x, y);
		}
	}
}

//	ENDSCREEN AND STUFF

bool Encounter::checkWinCondition() {
	bool ans = true;
	for (auto& it : profiles) if (it->sentience == AI) {
		if (!it->bDead)
			ans = false;
	}	return ans;
}
bool Encounter::checkLoseCondition() {
	CombatProfile* main = nullptr;
	for (auto& it : profiles) {
		MuteProfile* cast = dynamic_cast <MuteProfile*> (it);
		if (cast != nullptr) main = cast;
	}
	if (main == nullptr || main->bDead) return true;
	return false;
}
void Encounter::checkEndConditions() {
	if (checkWinCondition()) {
		endMessage = "    You emerge victorious! But for how long? :)";
	}
	else
		if (checkLoseCondition()) {
			endMessage = "    You lose - unsurprisingly.";
		}
		else return;

	freezeFight();
	new Timer(1.5f, this, &Encounter::endFight);
}

void Encounter::freezeFight() {
	bFreezeFight = true;
	for (auto& it : controllers) if (it) it->freeze();
	for (auto& it : menus) if (it) it->freeze();
	for (int i = 0; i < 4; ++i) if (menus[ACT]) {
		menus[ACT]->buttons[i].hoveredShape->setComponentColor(0, Color(GRAY));
		menus[ACT]->buttons[i].shape->setComponentColor(0, Color(GRAY));
	}	borders[ACT]->color = DWHITE;
	UIObj[pidx]->elem[namePrint]->hide();
	eventLogger->hide();

	UICtrl->freeze();
	Animator::setGlobalAnimSpeed(0.5f);
	new Timer(1.5f, &Animator::resetGlobalAnimSpeed);
}
void Encounter::endFight() {
	endMessageWin = new Narration(getwidth() / 2 - 35, getheight() / 2 - 14, 80, false, "narratorbox2");
	for (auto& it : profiles) {
		if (it->sentience == PLAYER)
			it->ch->requestAnimation("celebration", "idle");
	}
	endMessageWin->playText(endMessage);
}

//	PROMPT STUFF

void Encounter::sendPromptAnswer(std::string message) {
}
void Encounter::setupAttackInteraction(CombatProfile* src, CombatProfile* target, CombatMoveInterface* move) {
}

void Encounter::runPromptTasks() {
	if (commandPrompt == nullptr)
		commandPrompt = createPrompt();
	for (auto ctrl : controllers) ctrl->freeze();

	if (!commandPrompt->isPromptAlive()) {
		sendPromptAnswer(commandPrompt->answer);
		bPrompted = false;
		commandPrompt->destroy();
		bCanPrompt = false;
		new Timer(0.2f, this, &Encounter::refreshPrompt);

		commandPrompt = nullptr;
		wakeUpControllers();
	}	escapeCheck->wakeUp();
}

PromptAsker* Encounter::createPrompt() { return new PromptAsker(new Prompt("prompt", getwidth() / 2 - 40, getheight() / 2 - 4 - 11, false, 80)); }

//	UI

void Encounter::updateUI(float delta) {
	if (UIObj.empty()) {
		if (profiles.empty()) return;
		else setupUI();
	}

	int HPWidth = 15;
	for (int i = 0, k = 0; k < (int)profiles.size(); ++k) {
		auto& profile = profiles[k];
		if (profile->sentience == OBJECT) continue;
		float scale;
		if (profile->HP >= 0)	scale = (std::min)(1.0f, (float)profile->HP / profile->init->HP);
		else					scale = 0;
		UIObj[i]->elem[HPBar]->updateShape(std::string((size_t)(std::max)((scale == 0 ? .0f : 1.0f), HPWidth * scale), ':'));
		UIObj[i]->elem[nameDown]->updateShape(profile->name);
		UIObj[i]->elem[HPDown]->updateShape(profile->hpString());

		if (profile->bDead) UIObj[i]->elem[nameDown]->color = DRED;
		else				UIObj[i]->elem[nameDown]->color = WHITE;
		UIObj[i]->elem[HPPrint]->updateShape(" " + profile->hpString());
		UIObj.back()->elem[MPPRINT]->updateShape(" " + profile->mpString());
		UIObj.back()->elem[APPRINT]->updateShape(profile->apString());

		UIObj.back()->elem[WPrint]->updateShape("DR: " + intToStr(profile->DR) + "%");

		UIObj[i]->elem[namePrint]->hide();
		UIObj[i]->elem[HPPrint]->hide();
		UIObj[i]->elem[MPPRINT]->hide();
		UIObj[i]->elem[APPRINT]->hide();
		UIObj[i]->elem[statsBorder]->hide();

		UIObj[i]->elem[WPrint]->hide();
		UIObj[i]->elem[LogCtrlPrint]->hide();
		UIObj[i]->elem[LogCtrlPrint2]->hide();
		UIObj[i]->elem[WindowCtrlPrint]->hide();
		UIObj[i]->elem[WPrint]->hide();
		UIObj[i]->elem[DWPrint]->hide();
		UIObj[i]->elem[APRegen]->hide();
		UIObj[i]->elem[MPRegen]->hide();

		++i;
	}	if (currentProfile()->sentience == OBJECT) return;

	UIObj[pidx]->elem[namePrint]->show();
	UIObj[pidx]->elem[HPPrint]->show();
	UIObj[pidx]->elem[MPPRINT]->show();
	UIObj[pidx]->elem[APPRINT]->show();

	if (currentProfile()->sentience == AI) {
		UIObj[pidx]->elem[HPPrint]->hide();
		UIObj[pidx]->elem[MPPRINT]->hide();
		UIObj[pidx]->elem[APPRINT]->hide();
	}

	UIObj[pidx]->elem[nameDown]->color = YELLOW;
	if (currentProfile()->attackTarget && bInSelection && menus[ACT]->hover == 0 && menuSelector) {
		for (int i = 0; i < (int)profiles.size(); ++i)
			if (profiles[i] == currentProfile()->attackTarget)
				UIObj[i]->elem[nameDown]->color = RED;
	}

	if (bPrintExtendedStats) for (int j = 0; j < 7; ++j)
		UIObj[pidx]->elem[WPrint]->show(),
		UIObj[pidx]->elem[LogCtrlPrint]->show(),
		UIObj[pidx]->elem[LogCtrlPrint2]->show(),
		UIObj[pidx]->elem[WindowCtrlPrint]->show(),
		UIObj[pidx]->elem[APRegen]->show(),
		UIObj[pidx]->elem[MPRegen]->show(),
		UIObj[pidx]->elem[DWPrint]->show(),
		UIObj[pidx]->elem[statsBorder]->show();
}
void Encounter::setupUI() {
	int HPWidth = 15;
	int width = 25;
	for (int i = 0, k = 0; k < (int)profiles.size(); ++k, ++i) {
		auto& profile = profiles[k];
		if (profile->sentience == OBJECT) continue;
		int x = 2 + i * width;
		int y = getheight() - 3;
		UIObj.push_back(new individualUIElements());
		UIObj.back()->elem[HPBar] = new ScreenObject(x + 1, y + 1, 0);
		UIObj.back()->elem[HPBarMissing] = new ScreenObject(x + 1, y + 1, -1);
		UIObj.back()->elem[brackets] = new ScreenObject(x, y + 1, -2);
		UIObj.back()->elem[HPDown] = new ScreenObject(x, y, 50);
		UIObj.back()->elem[nameDown] = new ScreenObject(x + (profile->hpString().size() + 1), y, 50);
		UIObj.back()->elem[HPPrint] = new ScreenObject(5, 2, 105);
		UIObj.back()->elem[MPPRINT] = new ScreenObject(45, 2, 105);
		UIObj.back()->elem[APPRINT] = new ScreenObject(25, 2, 105);
		UIObj.back()->elem[namePrint] = new ScreenObject(5, 0, 105);
		UIObj.back()->elem[statsBorder] = new ScreenObject(68, -2, 150, "fake_window");

		UIObj.back()->elem[HPBar]->color = GREEN;
		UIObj.back()->elem[HPBarMissing]->color = DRED;
		UIObj.back()->elem[brackets]->color = WHITE;
		UIObj.back()->elem[HPPrint]->color = RED;
		UIObj.back()->elem[MPPRINT]->color = DGREEN;
		UIObj.back()->elem[APPRINT]->color = CYAN;

		UIObj.back()->elem[HPBar]->updateShape(std::string(HPWidth, ':'));
		UIObj.back()->elem[HPBarMissing]->updateShape(std::string(HPWidth, ':'));
		UIObj.back()->elem[brackets]->updateShape('[' + std::string(HPWidth, ' ') + ']');
		UIObj.back()->elem[nameDown]->updateShape(profile->name);
		UIObj.back()->elem[HPDown]->updateShape(profile->hpString());

		UIObj.back()->elem[namePrint]->updateShape(profile->name + "'s turn - (W/S) to select an action!");
		UIObj.back()->elem[HPPrint]->updateShape(" " + profile->hpString());
		UIObj.back()->elem[MPPRINT]->updateShape(" " + profile->mpString());
		UIObj.back()->elem[APPRINT]->updateShape(profile->apString());

		x = 2 + 102 - 29; y = 1;

		UIObj.back()->elem[WPrint] = new ScreenObject(x, y, 200);
		UIObj.back()->elem[WPrint]->updateShape("DR: " + intToStr(profile->DR) + "%");
		UIObj.back()->elem[WPrint]->color = WHITE;
		UIObj.back()->elem[WPrint]->setPosition(x, y);
		++y, ++x;

		UIObj.back()->elem[APRegen] = new ScreenObject(x, y, 200);
		UIObj.back()->elem[APRegen]->updateShape("AP Regen: " + intToStr(profile->regenAP) + " ");
		UIObj.back()->elem[APRegen]->color = WHITE;
		UIObj.back()->elem[APRegen]->setPosition(x, y);
		++y, ++x;

		UIObj.back()->elem[MPRegen] = new ScreenObject(x, y, 200);
		UIObj.back()->elem[MPRegen]->updateShape("MP Regen: " + intToStr(profile->regenMP) + " ");
		UIObj.back()->elem[MPRegen]->color = WHITE;
		UIObj.back()->elem[MPRegen]->setPosition(x, y);
		++y, ++x;

		++y, ++x;
		UIObj.back()->elem[DWPrint] = new ScreenObject(x, y, 200);
		UIObj.back()->elem[DWPrint]->updateShape("press E - shortcut to end turn\n");
		UIObj.back()->elem[DWPrint]->color = GRAY;
		UIObj.back()->elem[DWPrint]->setPosition(x, y);
		++x, ++y;

		UIObj.back()->elem[LogCtrlPrint] = new ScreenObject(x, y, 200);
		UIObj.back()->elem[LogCtrlPrint]->updateShape("hold I, J, K, L, C - log controls");
		UIObj.back()->elem[LogCtrlPrint]->color = GRAY;
		UIObj.back()->elem[LogCtrlPrint]->setPosition(x, y);
		++x, ++y;

		UIObj.back()->elem[LogCtrlPrint2] = new ScreenObject(x, y, 200);
		UIObj.back()->elem[LogCtrlPrint2]->updateShape("press UP/DOWN key - log scroll");
		UIObj.back()->elem[LogCtrlPrint2]->color = GRAY;
		UIObj.back()->elem[LogCtrlPrint2]->setPosition(x, y);
		++x, ++y;

		UIObj.back()->elem[WindowCtrlPrint] = new ScreenObject(x, y, 200);
		UIObj.back()->elem[WindowCtrlPrint]->updateShape("press X - hide window");
		UIObj.back()->elem[WindowCtrlPrint]->color = GRAY;
		UIObj.back()->elem[WindowCtrlPrint]->setPosition(x, y);
		++x, ++y;

		for (int i = 0; i < UI_COUNT; ++i) {
			if (UIObj.back()->elem[i]) UIObj.back()->elem[i]->setName("uielem");
		}
	}
}

void Encounter::flashHPBar(CombatProfile* profile) {
	if (profile->sentience == OBJECT) return;
	int idx = 0;
	while (profiles[idx] != profile) ++idx;
	new FlashBehaviour <ScreenObject>(3, 0.1f, 0.1f, UIObj[idx]->elem[HPBar]);
	new FlashBehaviour <ScreenObject>(3, 0.1f, 0.1f, UIObj[idx]->elem[HPBarMissing]);
}

//	LITTLE FUNCTIONS

CombatProfile* Encounter::currentProfile() { return profiles[pidx]; }
std::vector <Character*> Encounter::currentMoveTargets() { 
	std::vector <Character*> ans;
	for (auto cp : currentMove->targetsInRange(currentProfile()->ch, getCharacters()))
		ans.push_back(cp->ch);
	return ans;
}
std::vector <CombatProfile*> Encounter::currentMoveProfiles() {
	std::vector <CombatProfile*> profiles;
	for (auto it : currentMoveTargets()) profiles.push_back(it->cp);
	return profiles;
}

std::vector<CombatProfile*> Encounter::targetsInRange(CombatProfile* cp) {
	return cp->attackArr[0]->targetsInRange(cp->ch, characters);
}

std::vector<std::pair<int, int>> Encounter::getTilesInRange(int x, int y, int range) {
	std::vector<std::pair<int, int>> arr;
	for (int i = -range, j; i <= range; ++i)
		for (j = -(range - (std::max)(i, -i)); j <= range - (std::max)(i, -i); ++j)
			arr.push_back({ x + j, y + i });
	return arr;
}

CombatProfile* Encounter::getProfileByName(std::string name) {
	for (auto it : profiles) if (it->name == name) return it;
	return nullptr;
}
std::vector<struct ObjectProfile*> Encounter::getObjects() {
	std::vector <ObjectProfile*> ans;
	for (auto it : profiles) if (it->sentience == OBJECT)
		ans.push_back(dynamic_cast <ObjectProfile*> (it));
	return ans;
}

bool Encounter::checkValability(std::pair<int, int> pair) {
	if (LAYOUTS->selectTile(grid, pair) == -1) return false;
	for (auto& it : profiles) {
		if (it->getTile() == pair) return false;
	}	return true;
}
void Encounter::addCharacter(Character* ch) {
	characters.push_back(ch);
	profiles.push_back(ch->cp);
	new DeathDetection(ch->cp);
}
void Encounter::wakeUpControllers() {
	for (auto ctrl : controllers) if (ctrl) ctrl->wakeUp();
	if (menus[ACT]) menus[ACT]->wakeUp();
}

std::string Encounter::descriptionGen() {
	std::string ret = "";
	if (currentMoveOption) {
		ret = currentMoveOption->descriptionGen();
	}	return ret;
}

void Encounter::toggleLogVisibility() {
	if (eventLogger->bHidden) eventLogger->show();
	else					  eventLogger->hide();
}

std::pair<int, int> Encounter::centerPopUpHelper(int width) {
	int x, y;
	auto unit = currentProfile();
	x = grid.pivot_x[unit->grid_y][unit->grid_x];
	y = grid.pivot_y[unit->grid_y][unit->grid_x];
	y -= 16;
	x -= width / 2 - 8;
	x = (std::max)(0, x);
	x = (std::min)(getwidth() - width, x);
	return std::pair<int, int>(x, y);
}

void Encounter::centerPopUp() {
	if (arrow) arrow->hide();
	if (bInMovement || currentProfile()->sentience == AI) {
		if (borders[POPUP]) borders[POPUP]->hide();
		else if (borders[SPOPUP]) borders[SPOPUP]->hide();
		return;
	}	int x = centerPopUpHelper(popup_width).first, y = centerPopUpHelper(popup_width).second;
	if (borders[POPUP]) {
		borders[POPUP]->setPosition(x, y);
		if (!borders[POPUP]->isHidden())
			arrow->setPosition(centerPopUpHelper(6).first, centerPopUpHelper(6).second + 7),
			arrow->show();
	}
	if (borders[SPOPUP]) {
		borders[SPOPUP]->setPosition(x, y);
		if (!borders[SPOPUP]->isHidden())
			arrow->setPosition(centerPopUpHelper(6).first, centerPopUpHelper(6).second + 4),
			arrow->show();
	}
}

void Encounter::centerHoverArrow(CombatProfile* hovered) {
	if (currentProfile()->sentience == AI) return;
	if (hover_arrow && hovered) {
		int x = hovered->grid_x, y = hovered->grid_y;
		hover_arrow->setPosition(grid.pivot_x[y][x], grid.pivot_y[y][x] - 16);
		hover_arrow->color = RED;
		anim_hover_arrow->setPosition(grid.pivot_x[y][x], grid.pivot_y[y][x] - 16);
		anim_hover_arrow->color = RED;
		if (bAttackTargetRequest || menus[ACT]->hover == 1) {
			anim_hover_arrow->show();
			if (!anim_hover_arrow->isPlaying()) anim_hover_arrow->reset(), anim_hover_arrow->play();
		}
		else hover_arrow->show(), anim_hover_arrow->pause(), anim_hover_arrow->hide();
	}
}

//	MENU/INPUT FUNCTIONS

void Encounter::createMenus() {
	if (menus[ACT]) {
		menus[ACT]->wakeUp();
		return;
	}	table = new GUI_WindowsTable("battle_menu", 80);

	borders[ACT] = new GUI_Window(105, "", x, y);
	borders[ACT]->setName("actMenu");
	borders[ACT]->color = WHITE;
	table->updateWindow(borders[ACT], "banner");
	menus[ACT] = new GUI_Menu("W", "S", x, y);
	menus[ACT]->bAutoDestroy = false;

	updateHistory <CombatBeginLog>(new Event());

	addController(statsCtrl = new Controller(1));
	statsCtrl->setToggleEvent(KEY_PRESS, "X", this, &Encounter::statsWindowToggle);
	statsWindowToggle();
	statsCtrl->wakeUp();

	addController(endTurnCtrl = new Controller(1));
	endTurnCtrl->setToggleEvent(KEY_PRESS, "E", this, &Encounter::endPlayerTurn);
	endTurnCtrl->wakeUp();

	addController(movementCtrl = new Controller(1));
	movementCtrl->setToggleEvent(KEY_PRESS, "A", this, &Encounter::moveTile <-1, 0>);
	movementCtrl->setToggleEvent(KEY_PRESS, "D", this, &Encounter::moveTile < 1, 0>);
	movementCtrl->setToggleEvent(KEY_PRESS, "W", this, &Encounter::moveTile < 0, -1>);
	movementCtrl->setToggleEvent(KEY_PRESS, "S", this, &Encounter::moveTile < 0, 1>);
	movementCtrl->setToggleEvent(KEY_PRESS, "ENTER", this, &Encounter::selectTile);

	addController(UICtrl = new Controller(1));
	UICtrl->setToggleEvent(KEY_PRESS, "Z", this, &Encounter::advanceAnimSpeedScale);
	UICtrl->wakeUp();

	delete table;
	int description_width = 80;
	log_x = (float)getwidth() - description_width, log_y = 0;
	table = new GUI_WindowsTable("battle_menu", description_width);
	descriptions = new GUI_Window(100, "", (int)log_x, (int)log_y);
	descriptions->setName("desc");
	table->updateWindow(descriptions, "box");
	descriptions->hide();
	descriptions->setFunction("0", STRDISPLAY, this, &Encounter::descriptionGen);
	descriptions->forceFormat();

	delete table;
	log_x = (float)getwidth() - 60, log_y = (float)getheight() - 13;
	log_x = 11, log_y = 10;
	log_x = (float)(getwidth() - 70), log_y = 14.0f;
	table = new GUI_WindowsTable("battle_menu", 65);
	GUI_Window* loggerWin = new GUI_Window(150, "", 0, 0);
	loggerWin->setName("loggerWin");
	table->updateWindow(loggerWin, "logbox");
	eventLogger = new LoggerWindow(loggerWin, (int)log_x, (int)log_y);
	eventLogger->hide();
	arrow = new ScreenObject(0, 0, 120, "battle_menu_arrow");
	arrow->setName("arrow");
	arrow->hide();
	hover_arrow = new ScreenObject(0, 0, 120, "battle_hover_arrow");
	hover_arrow->hide();
	hover_arrow->setName("hoverArr");
	anim_hover_arrow = new Animator("battle_hover_arrow", 5, 5, 1000, false);
	anim_hover_arrow->hide();
	addController(logCtrl = new Controller(1));
	logCtrl->wakeUp();
	logCtrl->setToggleEvent(KEY_PRESS, "C", this, &Encounter::toggleLogVisibility);

	logCtrl->setToggleEvent(KEY_PRESS, "K", this, &Encounter::setDown  <true>);
	logCtrl->setToggleEvent(KEY_RELEASE, "K", this, &Encounter::setDown  <false>);
	logCtrl->setToggleEvent(KEY_PRESS, "I", this, &Encounter::setUp    <true>);
	logCtrl->setToggleEvent(KEY_RELEASE, "I", this, &Encounter::setUp    <false>);
	logCtrl->setToggleEvent(KEY_PRESS, "J", this, &Encounter::setLeft  <true>);
	logCtrl->setToggleEvent(KEY_RELEASE, "J", this, &Encounter::setLeft  <false>);
	logCtrl->setToggleEvent(KEY_PRESS, "L", this, &Encounter::setRight <true>);
	logCtrl->setToggleEvent(KEY_RELEASE, "L", this, &Encounter::setRight <false>);
	logCtrl->setToggleEvent(KEY_PRESS, "UP", eventLogger, &LoggerWindow::scrollUp);
	logCtrl->setToggleEvent(KEY_PRESS, "DOWN", eventLogger, &LoggerWindow::scrollDown);

	delete table;
	table = new GUI_WindowsTable("battle_menu", popup_width);
	borders[POPUP] = new GUI_Window(106, "", dummyx, liney);
	borders[SPOPUP] = new GUI_Window(106, "", dummyx, liney);
	borders[POPUP]->setName("popup");
	borders[SPOPUP]->setName("spopup");
	table->updateWindow(borders[POPUP], "popup");
	table->updateWindow(borders[SPOPUP], "popup-small");
	borders[POPUP]->hide();
	borders[SPOPUP]->hide();

	delete table;
	table = new GUI_WindowsTable("battle_menu", 25);
	internal_addButtonRoutine(ACT, 0, &attackBanner, &attackBannerOnHover, &Encounter::attackPress, Color(), Color(RED, DARK));
	internal_addButtonRoutine(ACT, 1, &spellBanner, &spellBannerOnHover, &Encounter::spellPress, Color(), Color(CYAN, DARK));
	internal_addButtonRoutine(ACT, 2, &moveBanner, &moveBannerOnHover, &Encounter::movePress, Color(), Color(GREEN, DARK));
	internal_addButtonRoutine(ACT, 3, &commandBanner, &commandBannerOnHover, &Encounter::commandPress, Color(), Color(YELLOW, DARK));

	if (!currentProfile()->getSpells().empty()) menus[ACT]->buttons[1].hoveredShape->setComponentColor(0, Color(CYAN));
	else menus[ACT]->buttons[1].hoveredShape->setComponentColor(0, Color(GRAY));

	delete table;

	addController(escapeCheck = new Controller(5));
	escapeCheck->setToggleEvent(KEY_PRESS, "ESC", this, &Encounter::pressEscape);
	escapeCheck->wakeUp();
}
void Encounter::internal_addButtonRoutine(MenuTypes type, int idx, std::string(*func)(), std::string(*hfunc)(), void (Encounter::* buttonPress)(), Color base, Color hovered) {
	GUI_Window* win = new GUI_Window(105, "", 0, 0);
	win->setName("button");
	table->updateWindow(win, 0);
	win->setFunction("0", STRDISPLAY, func);
	win->color = base;
	win->setNewestComponentColor(base);
	GUI_Window* hover_win = new GUI_Window(105, "", 0, 0);
	hover_win->setName("button");
	table->updateWindow(hover_win, 1);
	hover_win->setFunction("0", STRDISPLAY, hfunc);
	hover_win->color = hovered;
	hover_win->setNewestComponentColor(hovered);
	menus[type]->addButton(win, hover_win, borders[type]->getFBoxPosition(idx).first, borders[type]->getFBoxPosition(idx).second, this, buttonPress);
}
bool Encounter::checkEscapePress() {
	bool ans = bPressedEscape;
	if (bPressedEscape) {
		escapeAction(), bPressedEscape = false;
		if (bAttackTargetRequest) bAttackTargetRequest = false, descriptions->hide(), currentMoveOption = nullptr;
	}	return ans;
}

void Encounter::escapeAction() {
	bInSelection = false;
	request = nullptr;
	if (bPrompted) {
		bPrompted = false;
		commandPrompt->cancel();
		if (commandPrompt) commandPrompt->destroy();
		bCanPrompt = false;
		new Timer(0.5f, this, &Encounter::refreshPrompt);
	}
	LAYOUTS->gridColor(grid, WHITE);
	if (menus[ACT]->hover == 0) {
		if (singleSelector) singleSelector->destroy(), singleSelector->menu->destroy(), singleSelector = nullptr, borders[POPUP]->hide();
		if (multipleSelector) multipleSelector->destroy(), multipleSelector = nullptr, borders[POPUP]->hide();
		if (menuSelector) menuSelector->destroy(), menuSelector->menu->destroy(), menuSelector = nullptr, borders[POPUP]->hide();
		characters[pidx]->requestAnimation("idle");
	}
	else
		if (menus[ACT]->hover == 1) {
			if (singleSelector) singleSelector->destroy(), singleSelector->menu->destroy(), singleSelector = nullptr, borders[POPUP]->hide();
			if (multipleSelector) multipleSelector->destroy(), multipleSelector = nullptr, borders[POPUP]->hide();
			if (menuSelector) menuSelector->destroy(), menuSelector->menu->destroy(), menuSelector = nullptr, borders[POPUP]->hide();
			characters[pidx]->requestAnimation("idle");
		}
		else
			if (menus[ACT]->hover == 2) {
				bTilesSelection = false;
				LAYOUTS->clearAnim(grid);
			}	menus[ACT]->wakeUp();
			if (borders[POPUP]) borders[POPUP]->clearComponents(), borders[POPUP]->hide();
}

void Encounter::attackPress() {
	auto ch = currentProfile();
	auto atk = *ch->attackArr.begin();
	if (atk->checkRequirements(currentProfile()));
	else { addErrorMessage("Attack requirements not met: " + atk->shortDescription()); return; }
	currentMoveOption = currentMove = atk;
	if (currentProfile()->sentience != AI)
		descriptions->show();
	bAttackTargetRequest = true;
	bInSelection = true;
	menus[ACT]->freeze();
	createSingleTargetInputSelector(targetsInRange(currentProfile()));
}

void Encounter::spellPress() {
	if (currentProfile()->getSpells().empty()) { addErrorMessage("Character has no skills to use!"); return; }
	std::vector <std::string> choices;
	for (int i = 0; i < (int)currentProfile()->spellArr.size(); ++i)
		choices.push_back(intToStr(i + 1) + ". " + currentProfile()->spellArr[i]->getName() + currentProfile()->spellArr[i]->getBonusInfo());
	int x = centerPopUpHelper(popup_width).first + 2;
	int y = centerPopUpHelper(popup_width).second + 2;
	menuSelector = new InputSingleSelector(choices, x, y, selector_settings(0, 1, 4, 28, false));
	borders[POPUP]->clearComponents();
	borders[POPUP]->setFunction("0", STRDISPLAY, &selectMove);
	menus[ACT]->freeze();
	bInSelection = true;
}

void Encounter::movePress() {
	if (profiles[pidx]->MP <= 0) {
		addErrorMessage("No MP left to move!");
		return;
	}
	if (currentProfile()->sentience == PLAYER && !bPlayerVulnerability);
	else LAYOUTS->paintTile(grid, currentProfile()->getVulnerableTile());
	int x = currentProfile()->grid_x, y = currentProfile()->grid_y;
	requestTileSelection(1, currentProfile()->MP, x, y, true);
	menus[ACT]->freeze();
	if (currentProfile()->sentience != AI)
		borders[SPOPUP]->show();
	borders[SPOPUP]->clearComponents();
	borders[SPOPUP]->setFunction("0", STRDISPLAY, &selectMove2);

	if (currentProfile()->sentience == AI) {
		if (borders[POPUP]) borders[POPUP]->hide();
		if (borders[SPOPUP]) borders[SPOPUP]->hide();
	}
}

void Encounter::commandPress() {
	endTurnPress();
}

void Encounter::endTurnPress() {
	endPlayerTurn();
}

//	SELECTION FUNCTIONS

void Encounter::combatMoveTargetRequestSetup(CombatMoveInterface* moveInterface, int count, bool bDistinct) {
	if (Encounter::getInstance()->bSimMode) {
		currentProfile()->brainAI->targetSetup(count, bDistinct, dynamic_cast <Spell*> (moveInterface));
		return;
	}
	if (count == 1) createSingleTargetInputSelector(currentMoveProfiles());
	else			createMultipleTargetInputSelector(currentMoveProfiles(), count, bDistinct);
	request = moveInterface;
}

void Encounter::selectTile() {
	bool bOccupied = false;
	for (auto it : profiles) if (it->grid_x == seltile.first && it->grid_y == seltile.second) bOccupied = true;
	if ((bRejectOccupiedTile || menus[ACT]->hover == 2) && bOccupied) { addErrorMessage("Tile already occupied."); return; }
	selectedTiles.push_back(grid.tiles[LAYOUTS->selectTile(grid, seltile)]);
}

void Encounter::createSingleTargetInputSelector(std::vector<CombatProfile*> profiles) {
	selectionChoices = profiles;
	bInSelection = true;
	std::vector <std::string> choices;
	for (auto it : profiles) {
		choices.push_back(it->name);
	}
	int x = centerPopUpHelper(popup_width).first + 2;
	int y = centerPopUpHelper(popup_width).second + 2;
	if (choices.size() == 0) {
		singleSelector = new InputSingleSelector(choices, x, y, selector_settings(0, 1, 15, 10, false));
		borders[POPUP]->clearComponents();
		borders[POPUP]->setFunction("0", STRDISPLAY, &noTargetsInAttackRange);

		if (currentProfile()->sentience == AI) {
			singleSelector->menu->bHide = true;
			if (borders[POPUP]) borders[POPUP]->hide();
			if (borders[SPOPUP]) borders[SPOPUP]->hide();
		}
		return;
	}
	singleSelector = new InputSingleSelector(choices, x, y, selector_settings(0, 1, 15, 10, false));
	borders[POPUP]->clearComponents();
	borders[POPUP]->setFunction("0", STRDISPLAY, &selectTarget);

	if (currentProfile()->sentience == AI) {
		singleSelector->menu->bHide = true;
		if (borders[POPUP]) borders[POPUP]->hide();
		if (borders[SPOPUP]) borders[SPOPUP]->hide();
	}

	if (currentProfile()->sentience == AI) solveSelector(singleSelector);
}

void Encounter::createMultipleTargetInputSelector(std::vector<CombatProfile*> profiles, int count, bool bDistinct) {
	selectionChoices = profiles;
	bInSelection = true;
	std::vector <std::string> choices;
	for (auto it : currentMoveTargets()) {
		choices.push_back(it->cp->name);
	}
	int x = centerPopUpHelper(popup_width).first + 2;
	int y = centerPopUpHelper(popup_width).second + 2;
	multipleSelector = new InputMultipleSelector(choices, count, x, y, bDistinct, selector_settings(0, 1, 15, 10, false));
	borders[POPUP]->clearComponents();
	borders[POPUP]->setFunction("0", STRDISPLAY, &selectTargets);

	if (currentProfile()->sentience == AI) {
		if (borders[POPUP]) borders[POPUP]->hide();
		if (borders[SPOPUP]) borders[SPOPUP]->hide();
	}
}

void Encounter::selectAttackSetup() {
	std::vector <std::string> choices;
	for (int i = 0; i < (int)currentProfile()->attackArr.size(); ++i)
		choices.push_back(intToStr(i + 1) + ". " + currentProfile()->attackArr[i]->getName());
	int x = centerPopUpHelper(popup_width).first + 2;
	int y = centerPopUpHelper(popup_width).second + 2;
	menuSelector = new InputSingleSelector(choices, x, y, selector_settings(0, 1, 15, 10, false));
	borders[POPUP]->clearComponents();
	borders[POPUP]->setFunction("0", STRDISPLAY, &selectMove);

	if (currentProfile()->sentience == AI) {
		if (borders[POPUP]) borders[POPUP]->hide();
		if (borders[SPOPUP]) borders[SPOPUP]->hide();
	}
	bInSelection = true;
}

void Encounter::requestTileSelection(int count, int range, int pivot_x, int pivot_y, bool bForbiddenRule) {
	selectedTiles.clear();
	if (bForbiddenRule) movementTiles = LAYOUTS->getTilesInRangeWithForbiddenRule(grid, range, pivot_x, pivot_y, movementTilesInPair);
	else movementTiles = LAYOUTS->getTilesInRange(grid, range, pivot_x, pivot_y, movementTilesInPair);
	this->count = count;
	seltile = { pivot_x, pivot_y };
	LAYOUTS->gridColor(grid, movementTilesInPair, Color(WHITE, DARK));

	std::vector <std::pair <int, float>> ganim;
	ganim.push_back({ 3, 0.5f });
	ganim.push_back({ 4, 0.5f });
	ganim.push_back({ 5, 0.5f });
	LAYOUTS->gridAnimate(grid, ganim, movementTiles, LOOP);
	bTilesSelection = true;
}

void Encounter::requestLineAttackTileSelection(CombatMoveInterface* moveInterface, int count, int range, int pivot_x, int pivot_y, bool bForbiddenRule, bool bOccupiedRule) {
	selectedTiles.clear();
	std::vector <std::pair <int, int>> tilepairs;
	for (int i = -range; i <= range; ++i)
		tilepairs.push_back({ pivot_x + i, pivot_y });
	if (bForbiddenRule) movementTiles = LAYOUTS->selectTiles(grid, tilepairs);
	else movementTiles = LAYOUTS->selectTiles(grid, tilepairs);

	for (auto p : tilepairs) {
		if (LAYOUTS->selectTile(grid, p) != -1)
			movementTilesInPair.push_back(p);
	}

	this->count = count;
	seltile = { pivot_x, pivot_y };
	LAYOUTS->gridColor(grid, movementTilesInPair, Color(WHITE, DARK));
	bRejectOccupiedTile = bOccupiedRule;

	std::vector <std::pair <int, float>> ganim;
	ganim.push_back({ 7, 0.5f });
	ganim.push_back({ 8, 0.5f });
	ganim.push_back({ 9, 0.5f });
	LAYOUTS->gridAnimate(grid, ganim, movementTiles, LOOP);
	bTilesSelection = true;
	bAttackTilesSelection = true;
	request = moveInterface;
}

void Encounter::requestAttackTileSelection(CombatMoveInterface* moveInterface, int count, int range, int pivot_x, int pivot_y, bool bForbiddenRule, bool bOccupiedRule) {
	selectedTiles.clear();
	if (bForbiddenRule) movementTiles = LAYOUTS->getTilesInRangeWithForbiddenRule(grid, range, pivot_x, pivot_y, movementTilesInPair);
	else movementTiles = LAYOUTS->getTilesInRange(grid, range, pivot_x, pivot_y, movementTilesInPair);
	this->count = count;
	seltile = { pivot_x, pivot_y };
	LAYOUTS->gridColor(grid, movementTilesInPair, Color(WHITE, DARK));
	bRejectOccupiedTile = bOccupiedRule;

	std::vector <std::pair <int, float>> ganim;
	ganim.push_back({ 7, 0.5f });
	ganim.push_back({ 8, 0.5f });
	ganim.push_back({ 9, 0.5f });
	LAYOUTS->gridAnimate(grid, ganim, movementTiles, LOOP);
	bTilesSelection = true;
	bAttackTilesSelection = true;
	request = moveInterface;
}

bool Encounter::checkAttackSelection() {
	bool bDone = false;
	auto atk = currentProfile()->attackArr[menuSelector->menu->hover];
	if (atk->checkRequirements(currentProfile())) {
		characters[pidx]->requestAnimation("attack", "idle");
		currentMove = atk;
		atk->castMove(currentProfile());
		bDone = true;
	}
	else addErrorMessage("Attack requirements not met."), bInSelection = true, menuSelector->resetChoice();
	return bDone;
}
bool Encounter::checkSpellsSelection() {
	bool bDone = false;
	auto sp = currentProfile()->spellArr[menuSelector->menu->hover];
	if (sp->checkRequirements(currentProfile())) {
		currentMove = sp;
		characters[pidx]->requestAnimation("cast");
		characters[pidx]->speak(Character::SPEAKTYPES::CAST, WHITE);
		sp->castMove(currentProfile());
		bDone = true;
	}
	else addErrorMessage("Spell requirements not met."), bInSelection = true, menuSelector->resetChoice();
	return bDone;
}
bool Encounter::finalizeSelection(std::string name) {
	selectionChoices.clear();
	for (auto it : profiles)
		if (it->name == name) {
			selectionChoices.push_back(it);
			break;
		}	bInSelection = false;
	communicateSelection();
	return true;
}
bool Encounter::finalizeSelection(std::vector<std::string> names) {
	selectionChoices.clear();
	for (auto name : names) for (auto it : profiles)
		if (it->name == name) {
			selectionChoices.push_back(it);
			break;
		}	bInSelection = false;
	communicateSelection();
	return true;
}

void Encounter::communicateSelection() {
	if (bTilesSelection) {
		if (bAttackTilesSelection) {
			bTilesSelection = false;
			bAttackTilesSelection = false;
			std::vector <std::pair <int, int>> tiles;
			tiles.push_back(seltile);
			request->triggerEvents(tiles);
			LAYOUTS->clearAnim(grid);
			return;
		}

		Event* event = new Event();
		event->src = currentProfile();
		updateHistory <MovementLog>(event);
		bTilesSelection = false;
		bInMovement = true;
		if (currentProfile()->grid_x <= seltile.first) currentProfile()->pres.movementDirection = 0;
		else currentProfile()->pres.movementDirection = 1;
		int movementCost = 0;
		new CombatGridLineTraceBehaviour <Character>(LAYOUTS->getPath(grid, currentProfile()->getTile(), seltile, movementCost), characters[pidx], 20 * animSpeed[animSpeedScale]);
		currentProfile()->applyMovementCost(movementCost);
		currentProfile()->grid_y = seltile.second;
		currentProfile()->grid_x = seltile.first;
		return;
	}
	if (bAttackTargetRequest) {
		bAttackTargetRequest = false;
		castAttack(selectionChoices.back());
	}

	if (request) {
		characters[pidx]->requestAnimation("idle");
		request->triggerEvents(selectionChoices), request = nullptr;
	}
	selectionChoices.clear();
}

void Encounter::runSelections() {
	if (singleSelector && singleSelector->wasDestroyed() == false) {
		if (request) {
			LAYOUTS->gridColor(grid, WHITE);
			CombatProfile* hovered = getProfileByName(singleSelector->getHoveredChoice());
			if (hovered) LAYOUTS->gridColor(grid, request->effectTiles(hovered->grid_x, hovered->grid_y), request->getColor());
		}
		if (singleSelector) centerHoverArrow(getProfileByName(singleSelector->getHoveredChoice()));
		if (singleSelector->choice != "") {
			LAYOUTS->gridColor(grid, WHITE);
			bInSelection = false;
			if (finalizeSelection(singleSelector->choice))
				singleSelector->destroy(), singleSelector->menu->destroy(), singleSelector = nullptr, borders[POPUP]->hide();
		}
	}
	else
		if (multipleSelector && multipleSelector->wasDestroyed() == false) {
			if (request && multipleSelector->selector()) {
				LAYOUTS->gridColor(grid, WHITE);
				CombatProfile* hovered = getProfileByName(multipleSelector->selector()->getHoveredChoice());
				LAYOUTS->gridColor(grid, request->effectTiles(hovered->grid_x, hovered->grid_y), request->getColor());
			}
			if (multipleSelector->selector()) centerHoverArrow(getProfileByName(multipleSelector->selector()->getHoveredChoice()));
			if (multipleSelector->choices.size() == multipleSelector->count) {
				LAYOUTS->gridColor(grid, WHITE);
				bInSelection = false;
				if (finalizeSelection(multipleSelector->choices))
					multipleSelector->destroy(), multipleSelector = nullptr, borders[POPUP]->hide();
			}
		}
		else
			if (menuSelector && menuSelector->wasDestroyed() == false) {
				if (currentProfile()->sentience != AI)
					descriptions->show();
				LAYOUTS->gridColor(grid, WHITE);
				if (menus[ACT]->hover == 0) {
					currentMoveOption = currentProfile()->attackArr[menuSelector->menu->hover];
					if (currentProfile()->attackTarget) centerHoverArrow(currentProfile()->attackTarget);
					int x = currentProfile()->attackTarget->grid_x;
					int y = currentProfile()->attackTarget->grid_y;
					LAYOUTS->gridColor(grid, currentMoveOption->effectTiles(x, y), currentMoveOption->getColor());
				}
				if (menus[ACT]->hover == 1) currentMoveOption = currentProfile()->spellArr[menuSelector->menu->hover];
				if (currentMoveOption) {
					int x = currentProfile()->grid_x;
					int y = currentProfile()->grid_y;
					if (currentMoveOption->isGlobal());
					else LAYOUTS->gridColor(grid, currentMoveOption->rangeTiles(x, y), currentMoveOption->getColor());
				}
				if (menuSelector->choice != "") {
					if (menus[ACT]->hover == 0) {
						bInSelection = false;
						if (checkAttackSelection()) LAYOUTS->gridColor(grid, WHITE), menuSelector->destroy(), menuSelector->menu->destroy(), menuSelector = nullptr, borders[POPUP]->hide(), bAttacked = true;
					}
					else
						if (menus[ACT]->hover == 1) {
							if (currentMoveOption->checkMoveValidity(characters[pidx], characters) != VC_OK) {
								addErrorMessage("No valid targets!");
								menuSelector->resetChoice();
								return;
							}	bInSelection = false;
							if (checkSpellsSelection()) LAYOUTS->gridColor(grid, WHITE), menuSelector->destroy(), menuSelector->menu->destroy(), menuSelector = nullptr, borders[POPUP]->hide();
						}
				}
			}
			else bInSelection = false;
}

void Encounter::runTileSelection() {
	menus[ACT]->freeze();
	LAYOUTS->gridColor(grid, movementTilesInPair, Color(WHITE, DARK));
	LAYOUTS->gridColor(grid, std::vector <std::pair <int, int>>(1, seltile), Color(GREEN, DARK));
	if (movementCtrl) movementCtrl->wakeUp();
	if ((int)selectedTiles.size() >= count) {
		communicateSelection();
		bPressedEscape = true;
	}
}

void Encounter::castAttack(CombatProfile* target) {
	currentProfile()->attackTarget = target;

	auto atk = currentMove;
	setupAttackInteraction(currentProfile(), target, atk);
	characters[pidx]->requestAnimation("attack", "idle");
	atk->castMove(currentProfile());

	currentMove = nullptr;
	descriptions->hide();
}

//	EVENTS DEFINITIONS

Event::Event() { src = dest = nullptr; }
void Event::call() {}

EventTrigger::EventTrigger(Event* event) { this->effect.event = event; Encounter::getInstance()->addEventTrigger(this); bFired = 0; }
bool EventTrigger::check(const std::vector<Encounter::logged_string>& rawHistory) {
	return false;
}
void EventTrigger::fire() {
	if (bFired) return;
	if (effect.event != nullptr) Encounter::getInstance()->addEventOnStack(effect.event);
	bFired = true;
}

std::string TeleportLET::genDescription(Event* event) {
	TeleportEvent* cast = dynamic_cast <TeleportEvent*> (event);
	if (cast == nullptr) return "UNKNOWN ERROR! This shouldn't happen...";
	return event->src->name + " has teleported, switching positions on the grid!";
}
std::string MoveCritLET::genDescription(Event* event) { return event->src->lastMove->getName() + " rolled positive for a crit!"; }

std::string FixedDamageLET::genDescription(Event* event) {
	FixedDamageEvent* cast = dynamic_cast <FixedDamageEvent*> (event);
	if (cast == nullptr) return "UNKNOWN ERROR! This shouldn't happen...";
	if (event->src == event->dest) {
		return event->bonus + event->src->name + " dealt " + intToStr(cast->getValue()) + " damage to himself.";
	}
	return event->bonus + event->src->name + " dealt " + intToStr(cast->getValue()) + " damage to " + event->dest->name + ".";
}

std::string TrueScallingMAXHPDamageLET::genDescription(Event* event) {
	TrueScallingMAXHPDamageEvent* cast = dynamic_cast <TrueScallingMAXHPDamageEvent*> (event);
	if (cast == nullptr) return "UNKNOWN ERROR! This shouldn't happen...";
	if (event->src == event->dest) {
		return event->bonus + event->src->name + " dealt " + intToStr(cast->getValue()) + " damage to himself.";
	}
	return event->bonus + event->src->name + " dealt " + intToStr(cast->getValue()) + " damage to " + event->dest->name + ".";
}

std::string DeathEventLET::genDescription(Event* event) {
	return event->dest->name + " has fallen.";
}
std::string KillRewardLET::genDescription(Event* event) {
	return "Thrill of the kill! " + event->src->name + " received bonuses for succesfully killing his target.";
}

//	AI STUFF

void Encounter::turnOffSim() {
	bSimMode = false;
	for (auto it : simArr) delete it;
	simArr.clear();
}

void Encounter::runAITasks() {
	auto& brain = currentProfile()->brainAI;
	if (brain->empty())
		brain->runCombatAI();
	else {
		int v = brain->front();
		brain->pop();

		if (v == -1) brain->actionMinus();
		else if (v == 0) brain->actionZero();
		else if (v == 1) brain->actionOne();
		else if (v == 2) brain->actionTwo();

		addAIDelay(0.5f);
	}
}

void Encounter::addAIDelay(float time) {
	pauseAI();
	if (aiTimer == nullptr || aiTimer->wasDestroyed()) {
		aiTimer = new Timer(time, this, &Encounter::restartAI);
	}
	else {
		aiTimer->resetTime();
	}
}

void Encounter::shakeVisualEffect(float time, float iter) {
	new ShakeBehaviour <GridTile> (time, iter, grid.tiles);
}

void Encounter::pressAttack(CombatProfile* target) {
	menus[ACT]->hover = 0;
	attackPress();
	finalizeSelection(target->name);
	singleSelector->destroy(), singleSelector->menu->destroy(), singleSelector = nullptr, borders[POPUP]->hide();
}
void Encounter::pressSkills() {
	menus[ACT]->hover = 1;
	spellPress();
	bInSelection = false;
	for (int i = 0; i < 100; ++i) {
		if (currentProfile()->brainAI->getCandidateSpell() == currentProfile()->spellArr[i]) {
			menuSelector->menu->hover = i;
			i = 100;
		}
	}
	currentProfile()->brainAI->getCandidateSpell()->checkMoveValidity(currentProfile()->ch, characters);
	if (checkSpellsSelection()) LAYOUTS->gridColor(grid, WHITE), menuSelector->destroy(), menuSelector->menu->destroy(), menuSelector = nullptr, borders[POPUP]->hide();
}

void Encounter::pressMovement() {
	menus[ACT]->hover = 2;
	movePress();
}
void Encounter::pressEndTurn() {
	endTurnPress();
}
void Encounter::pressTile(std::pair<int, int> pair) {
	seltile = pair;
	selectTile();
}
void Encounter::solveSelector(InputSingleSelector* selector) {
	currentProfile()->brainAI->solveSelector(selector);
}

//	ENCOUNTERS CONSTRUCTION

ForestBanditsFight::ForestBanditsFight(std::vector <Character*> characters) : Encounter("banditstutorial", characters) {
	profiles[0]->grid_x = 9;
	profiles[0]->grid_y = 4;
	profiles[1]->grid_x = 1;
	profiles[1]->grid_y = 0;
	profiles[2]->grid_x = 1;
	profiles[2]->grid_y = 8;
	profiles[3]->grid_x = 18;
	profiles[3]->grid_y = 8;
	profiles[4]->grid_x = 18;
	profiles[4]->grid_y = 0;

	for (auto& it : characters) it->requestAnimation("idle");

	for (int i = 0; i < (int)profiles.size(); ++i) {
		int x = profiles[i]->vulnerable_x;
		int y = profiles[i]->vulnerable_y;
		if (profiles[i]->sentience == PLAYER && !bPlayerVulnerability);
		else LAYOUTS->paintTile(grid, profiles[i]->getVulnerableTile(), 6);
		new DeathDetection(profiles[i]);
	}
}

std::string HealEventLET::genDescription(Event* event) {
	HealEvent* cast = dynamic_cast <HealEvent*> (event);
	if (cast == nullptr) return "UNKNOWN ERROR! This shouldn't happen...";
	return event->bonus + event->dest->name + " has been healed for " + intToStr(cast->getValue()) + " HP.";
}

DiningFight::DiningFight(std::vector<Character*> characters) : Encounter("diningfight", characters) {
	profiles[0]->grid_x = 1;
	profiles[0]->grid_y = 3;

	profiles[1]->grid_x = 19;
	profiles[1]->grid_y = 5;

	profiles[2]->grid_x = 18;
	profiles[2]->grid_y = 5;

	profiles[3]->grid_x = 18;
	profiles[3]->grid_y = 1;

	/*profiles[4]->grid_x = 19;
	profiles[4]->grid_y = 1;

	profiles[5]->grid_x = 19;
	profiles[5]->grid_y = 2;*/

	for (auto& it : characters) it->requestAnimation("idle");

	for (int i = 0; i < (int)profiles.size(); ++i) {
		int x = profiles[i]->vulnerable_x;
		int y = profiles[i]->vulnerable_y;
		if (profiles[i]->sentience == PLAYER && !bPlayerVulnerability);
		else LAYOUTS->paintTile(grid, profiles[i]->getVulnerableTile(), 6);
		new DeathDetection(profiles[i]);
	}
}

TrainingFight::TrainingFight(std::vector<Character*> characters) : Encounter("training", characters) {
	profiles[0]->grid_x = 1;
	profiles[0]->grid_y = 3;

	profiles[1]->grid_x = 1;
	profiles[1]->grid_y = 5;

	profiles[2]->grid_x = 1;
	profiles[2]->grid_y = 1;

	profiles[3]->grid_x = 17;
	profiles[3]->grid_y = 1;

	profiles[4]->grid_x = 1;
	profiles[4]->grid_y = 7;

	/*profiles[5]->grid_x = 19;
	profiles[5]->grid_y = 2;*/

	for (auto& it : characters) it->requestAnimation("idle");

	for (int i = 0; i < (int)profiles.size(); ++i) {
		int x = profiles[i]->vulnerable_x;
		int y = profiles[i]->vulnerable_y;
		if (profiles[i]->sentience == PLAYER && !bPlayerVulnerability);
		else LAYOUTS->paintTile(grid, profiles[i]->getVulnerableTile(), 6);
		new DeathDetection(profiles[i]);
	}

	bWin = bLose = false;
}

std::string EncourageEventLET::genDescription(Event* event) {
	EncourageEvent* cast = dynamic_cast <EncourageEvent*> (event);
	if (cast == nullptr) return "UNKNOWN ERROR! This shouldn't happen...";
	return event->bonus + event->dest->name + " was encouraged, gaining 3AP and 4MP!";
}

bool TrainingFight::checkWinCondition() {
	return bWin;
}

bool TrainingFight::checkLoseCondition() {
	return bLose;
}
