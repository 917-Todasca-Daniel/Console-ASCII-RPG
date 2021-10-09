#include "GarbageTruck.h"

#include "GodClass.h"

GarbageTruck::GarbageTruck() {
	GOD->addToGarbageCollection(this);
}

GarbageTruck::~GarbageTruck() {
	GOD->removeFromGarbageCollection(this);
}
