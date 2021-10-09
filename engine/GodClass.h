#ifndef GODCLASS_H
#define GODCLASS_H

#include "InmutableRenderer.h"
#include "ControllerManager.h"
#include "AudioManager.h"

#define GOD     GodClass::getInstance()
#define AUDIO   GodClass::getInstance()->getAudioManager()
#define coutput std::cout

#define DEBUG	GodClass::getInstance()->getDebugger()

#define FPS 60
#define FPS_UNIT (1.0f/(float)FPS)

class PrimitiveObject;
class VisualObject;
class GarbageTruck;
class Debugger;

class GodClass
{
public:
	~GodClass();

	static GodClass* getInstance() {
		if (!instance) {
			instance = new GodClass();
			instance->renderer = new InmutableRenderer();
			instance->ctrlManager = new ControllerManager();
			instance->audioManager = new AudioManager();
		}   return instance;
	}

	inline void setDebugger(Debugger *debugger) { this->debugger = debugger; }
	inline Debugger* getDebugger() { return debugger; }

	inline void addToGarbageCollection(GarbageTruck* obj) { garbageTracked.push_back(obj); }
	inline void removeFromGarbageCollection(GarbageTruck* obj) { garbageTracked.remove(obj); }

	void addObject (PrimitiveObject* object);

	inline ConsoleRenderer*   getRenderer()          const { return renderer; }
	inline AudioManager*      getAudioManager()      const { return audioManager; }
	inline ControllerManager* getControllerManager() const { return ctrlManager; }

	inline float getGametime() { return gametime; }

	inline void setTimeDilation(float value) { this->timeDilation = value; }

	void freeDelayedMemory();

protected:
	bool bPause;

	static GodClass* instance;
	float gametime;
	float timeDilation;

	Debugger *debugger;

	ConsoleRenderer*   renderer;
	ControllerManager* ctrlManager;
	AudioManager*	   audioManager;
	std::list <PrimitiveObject*> aliveObjects;
	std::list <GarbageTruck*>    garbageTracked;

	int delidx;
	float deltime;
	std::vector <PrimitiveObject*> deletes[2];

private:
	GodClass();

public:
	void loop(float delta);

	friend class Debugger;
};

#endif // GODCLASS_H
