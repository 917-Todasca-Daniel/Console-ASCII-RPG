#include "ControllerManager.h"

#include "Controller.h"

#include <algorithm>

ControllerManager::ControllerManager() {
	bDisableControllers = false;
}

void ControllerManager::addController(Controller* ctrl) {
	controllerList.push_back(ctrl);
}

void ControllerManager::freezeLowerPriorities(int bound) {
	for (auto it = controllerList.begin(); it != controllerList.end(); ++it)
		if (*it && (*it)->priority < bound)
			(*it)->freeze();
}

void ControllerManager::wakeControllers() {
	for (auto it = controllerList.begin(); it != controllerList.end(); ++it)
		if (*it) (*it)->wakeUp();
}

void ControllerManager::disableControllers() {
	bDisableControllers = true;
}
void ControllerManager::enableControllers() {
	bDisableControllers = false;
}

void ControllerManager::manage(float delta) {
	bset.clear(); bset.resize(Controller::keysNumber, 0);
	std::vector <std::pair <int, Controller*>> sorter;
	for (auto it : controllerList) if (it && it->bActive)
		sorter.push_back({ -it->priority, it });
	std::sort(sorter.begin(), sorter.end());
	for (auto it : sorter)
		if (it.second && it.second->bActive) it.second->checkInput(bset, delta);
	controllerList.clear();
}
