#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "PrimitiveObject.h"
#include "Delegates.h"

#include <vector>
#include <set>

enum KEYSTATE { KEY_PRESS = 0, KEY_RELEASE, KEY_HOLD };

class ControllerManager;
template <typename type>                 class PrimitiveClassMethodDelegate;
template <typename type, typename param> class ParameteredPrimitiveClassMethodDelegate;

class Controller : public PrimitiveObject
{
public:
	static int keysNumber;
	
	Controller(int priority = 0);

	virtual void destroy() override;

	void setToggleEvent(KEYSTATE state, std::string key, void (*event)(void));
	void setHeldEvent(std::string key, void (*event)(float));

	template <typename type> void setToggleEvent(KEYSTATE state, std::string key, type *obj, void (type::* event)(void));
	template <typename type> void setHeldEvent(std::string key, type *obj, void (type::* event)(float));

	void callToggleEvent(KEYSTATE state, int label);
	void callHeldEvent(float delta, int label);

	void freeze() { bActive = 0; }
	void wakeUp() { bActive = 1; }

protected:
	virtual ~Controller();

	virtual void loop(float delta) override;
	
	bool getKey(std::string str, int& ret);
	virtual void checkInput(std::vector <bool> bset, float delta);

	union toggleCallbackUnion {
		void (*ptr)(void);
		PrimitiveClassMethodDelegate <void>* delegate;
	};
	union heldCallbackUnion {
		void (*ptr)(float);
		ParameteredPrimitiveClassMethodDelegate <void, float>* delegate;
	};

	bool bActive;

	std::vector <toggleCallbackUnion> toggleEvents[2];
	std::vector <heldCallbackUnion>   heldEvents;

	std::set    <int>  controlledKeys;
	std::vector <bool> bPressed;
	std::vector <char> bDelegated[3];

private:
	int priority;

	friend class ControllerManager;

	friend class Debugger;
};

template <typename type>
void Controller::setToggleEvent(KEYSTATE state, std::string key, type *obj, void (type::* event)(void)) {
	int v;
	if (getKey(key, v)) {
		bDelegated[state][v] = 1;
		controlledKeys.insert(v);
		toggleEvents[state][v].delegate = new ClassMethodDelegate <void, type>(obj, event);
	}
}
template <typename type>
void Controller::setHeldEvent(std::string key, type *obj, void (type::* event)(float)) {
	int v;
	if (getKey(key, v)) {
		bDelegated[KEY_HOLD][v] = 1;
		controlledKeys.insert(v);
		heldEvents[v].delegate = new ParameteredClassMethodDelegate <void, type, float>(obj, event);
	}
}

class AlphaController : public Controller {
public:
	AlphaController();

	std::string str;
	bool        bEntered;

	void addCharacter(std::string key, char ch);

protected:
	std::vector <std::tuple <char, int, float>> chpairs;
	void checkInput(std::vector <bool> bset, float delta) override;

	float backspaceTime;

	friend class Debugger;
};

#endif // CONTROLLER_H
