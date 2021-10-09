#include "PrimitiveObject.h"

#include "stdc++.h"

#define	USE_NAMEMAP true

std::map <std::string, int> PrimitiveObject::nameMap	= std::map <std::string, int>();
std::set <std::string>		PrimitiveObject::notifiers	= std::set <std::string>();
int PrimitiveObject::obj_count = 0;

PrimitiveObject::PrimitiveObject() {
	parent = 0;
	rootName = "";
	setName("Primitive");
	GOD->addObject(this);
	lifespan = 0;
	bDestroyed = 0;
	bLoopPostcedence = 0;
}

PrimitiveObject::~PrimitiveObject() {
	eraseFromNameMapTable();
}

void PrimitiveObject::loop(float delta) {
	lifespan += delta;
}

void PrimitiveObject::eraseFromNameMapTable() {
	if (!USE_NAMEMAP) return;
	if (nameMap.find(rootName) == nameMap.end()) return;
	--nameMap[rootName];
	if (nameMap[rootName] == 0) nameMap.erase(rootName);
}

std::string PrimitiveObject::addToNameMapTable() {
	if (!USE_NAMEMAP) return "";
	if (nameMap.find(rootName) == nameMap.end()) nameMap[rootName] = -1;
	return rootName + intToStr(++ nameMap[rootName]);
}

void PrimitiveObject::sendNotifier(std::string notifier) { notifiers.insert(notifier); }
bool PrimitiveObject::checkNotifier(std::string notifier) {
	return notifiers.find(notifier) != notifiers.end();
}

void PrimitiveObject::removeNotifier(std::string notifier) {
	notifiers.erase(notifier);
}
