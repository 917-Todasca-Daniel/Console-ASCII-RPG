#include "Timer.h"

Timer::Timer(float duration, void (*event)()) : duration(duration) {
	callback = event;
	bDelegated = 0;
	setName("Timer");
}

Timer::~Timer() { if (bDelegated && delegate) delete delegate; }

void Timer::resetTime() {
	lifespan = 0;
}

void Timer::loop(float delta) {
	PrimitiveObject::loop(delta);
	if (lifespan > duration) {
		if (bDelegated) { if (delegate) delegate->call(); }
		else if (callback) callback();
		destroy();
	}
}

