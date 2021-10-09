#include "GodClass.h"

#include "GarbageTruck.h"
#include "VisualObject.h"
#include "Debugger.h"

#include "consolewizard.h"
#include "stdc++.h"

constexpr float DELSPAN = 3.0f;

GodClass* GodClass::instance = 0;
GodClass::GodClass() : gametime(0), timeDilation(1.0f) { 
	initialize(); 
	bPause = 0; 

	ctrlManager = nullptr;
	renderer = nullptr;
	audioManager = nullptr;
	debugger = nullptr;
	
	delidx = 0; 
	deltime = 0; 
}

GodClass::~GodClass() {
	auto it = aliveObjects.begin();
	while (it != aliveObjects.end()) if (*it) {
		auto obj = *it;
		it = aliveObjects.erase(it);
		delete obj;
	}

	delete renderer;
	delete ctrlManager;
	
	for (auto it : garbageTracked) if (it) delete it;
}

void GodClass::addObject(PrimitiveObject* object) {
	if (!object) return;
	aliveObjects.push_back(object);
}

void GodClass::freeDelayedMemory() {
	for (auto it : deletes[delidx]) if (it) delete it; deletes[delidx].clear();
	delidx = 1 - delidx;
	deltime = 0;
}

std::list <PrimitiveObject*> postcedent;
void GodClass::loop(float delta) {
	if (bPause) return;
	delta *= timeDilation;
	gametime += delta;

	for (auto& obj : aliveObjects)
		if (obj && !obj->bDestroyed) {
			if (obj->bLoopPostcedence) postcedent.push_back(obj);
			else obj->loop(delta);
		}
	
	for (auto& obj : postcedent)
		if (obj) obj->loop(delta);
	postcedent.clear();

	if (audioManager) audioManager->manage(delta);
	if (ctrlManager)  ctrlManager->manage(delta);
	if (renderer)     renderer->render(delta);

	auto it = aliveObjects.begin();
	while (it != aliveObjects.end())
		if (*it && (*it)->bDestroyed) {
			auto obj = *it;
			it = aliveObjects.erase(it);
			if (obj) {
				deletes[1-delidx].push_back(obj);
			}
		}	else ++it;

	deltime += delta;
	if (deltime > DELSPAN) freeDelayedMemory();
}
