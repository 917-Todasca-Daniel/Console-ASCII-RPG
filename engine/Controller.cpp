#include "Controller.h"

#include "GodClass.h"
#include "Debugger.h"

#include "consolewizard.h"

#include <map>
#include <tuple>

int Controller::keysNumber = 0;

static int count, max;
static int keys[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_BACK, VK_TAB, VK_RETURN, VK_SHIFT, VK_CONTROL, VK_MENU, VK_ESCAPE, VK_SPACE, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN,
0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, -1 };
static std::string names[] = { "LBUTTON", "RBUTTON", "MBUTTON", "BACKSPACE", "TAB", "ENTER", "SHIFT", "CTRL", "ALT", "ESC", "SPACE", "LEFT", "UP", "RIGHT", "DOWN",
"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "#"};
static char charr[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '#' };
static std::map <std::string, int> link;

bool isKeyPressed(int label) {
	if (GetKeyState(keys[label]) & 0x8000) return true;
	return false;
}

Controller::Controller(int priority) : priority(priority) {
	if (link.size() == 0) {
		count = max = 0;
		while (keys[count] != -1) {
			if (keys[count] + 1 > max)
				max = keys[count] + 1;
			link[names[count]] = count;
			++count;
		}
		Controller::keysNumber = max;
	}   setName("Controller");

	controlledKeys.clear();
	bPressed.resize(max, 0);
	for (int i = 0; i < 2; ++i)
		toggleEvents[i].resize(max, toggleCallbackUnion());
	heldEvents.resize(max, heldCallbackUnion());
	for (int i = 0; i < 3; ++i)
		bDelegated[i].resize(max, 2);

	freeze();
}
Controller::~Controller() {
	for (auto it : link) {
		for (int i = 0; i < 2; ++i)
			if (bDelegated[i][it.second] == 1)
				delete toggleEvents[i][it.second].delegate;
		if (bDelegated[KEY_HOLD][it.second] == 1)
			delete heldEvents[it.second].delegate;
	}
}

void Controller::destroy() {
	PrimitiveObject::destroy();
}

void Controller::loop(float delta) {
	if (parent && parent->wasDestroyed()) {
		destroy();
		return;
	}
	if (!bActive) return;
	PrimitiveObject::loop(delta);
	GOD->getControllerManager()->addController(this);
}

void Controller::checkInput(std::vector <bool> bset, float delta) {
	for (auto& it : controlledKeys) {
		if (bset[it]) continue;
		if (isKeyPressed(it)) {
			bset[it] = 1;
			if (!bPressed[it])
				callToggleEvent(KEY_PRESS, it);
			callHeldEvent(delta, it);
			bPressed[it] = 1;
		}
		else {
			if (bPressed[it])
				callToggleEvent(KEY_RELEASE, it);
			bPressed[it] = 0;
		}
	}
}

void Controller::callToggleEvent(KEYSTATE state, int label) {
	if (GOD->getControllerManager()->bDisableControllers) return;
	if (bDelegated[state][label] == 2) return;
	if (bDelegated[state][label]) {
		toggleEvents[state][label].delegate->call();
		return;
	}   toggleEvents[state][label].ptr();
}
void Controller::callHeldEvent(float delta, int label) {
	if (GOD->getControllerManager()->bDisableControllers) return;
	if (bDelegated[KEY_HOLD][label] == 2) return;
	if (bDelegated[KEY_HOLD][label]) {
		heldEvents[label].delegate->call(delta);
		return;
	}   heldEvents[label].ptr(delta);
}

void Controller::setToggleEvent(KEYSTATE state, std::string key, void (*event)(void)) {
	if (link.find(key) != link.end()) {
		int v = link[key];
		bDelegated[state][v] = 0;
		controlledKeys.insert(v);
		toggleEvents[state][v].ptr = event;
	}
}
void Controller::setHeldEvent(std::string key, void (*event)(float)) {
	if (link.find(key) != link.end()) {
		int v = link[key];
		bDelegated[KEY_HOLD][v] = 0;
		controlledKeys.insert(v);
		heldEvents[v].ptr = event;
	}
}
bool Controller::getKey(std::string str, int& ret) {
	if (link.find(str) == link.end()) return false;
	ret = link[str]; return true;
}


AlphaController::AlphaController() {
	str = "";
	bEntered = 0;
	backspaceTime = 0;
	int idx = 0, it = 0;
	while (names[it] != "A") {
		if (names[it] == "SPACE") chpairs.push_back(std::make_tuple(' ', it, 0.0f));
		++it;
	}
	while (charr[idx] != '#') chpairs.push_back(std::make_tuple(charr[idx], it, 0.0f)), ++idx, ++it;
}

void AlphaController::checkInput(std::vector <bool> bset, float delta) {
	Controller::checkInput(bset, delta);
	int idx = 0, it = 0;
	while (names[it] != "A") {
		if (names[it] == "ENTER") {
			if (isKeyPressed(it)) bEntered = 1;
		}
		else
			if (names[it] == "BACKSPACE") {
				if (isKeyPressed(it)) {
					bset[it] = 1;
					if (!bPressed[it])
						if (!str.empty()) str.pop_back();

					float& value = backspaceTime;
					value += delta;
					if (value > 0.5f) {
						if (!str.empty()) str.pop_back();
						value -= 0.02f;
					}
					bPressed[it] = 1;
				}
				else bPressed[it] = 0, backspaceTime = 0;
			}   ++it;
	}

	for (auto& pair : chpairs) {
		int it = std::get <1>(pair);
		if (isKeyPressed(it)) {
			bset[it] = 1;
			if (!bPressed[it])
				str += std::get <0>(pair);

			float& value = std::get <2>(pair);
			value += delta;
			if (value > 0.5f)
				str += std::get <0>(pair), value -= 0.02f;
			bPressed[it] = 1;
		}
		else {
			bPressed[it] = 0;
			std::get <2>(pair) = 0;
		}   ++idx, ++it;
	}
}

void AlphaController::addCharacter(std::string key, char ch) {
	int it = 0;
	while (names[it] != "#" && names[it] != key) ++it;
	if (names[it] == key) chpairs.push_back(std::make_tuple(ch, it, 0.0f));
}
