#ifndef CONTROLLERMANAGER_H
#define CONTROLLERMANAGER_H

#include <list>
#include <vector>

class Controller;

class ControllerManager
{
public:
	ControllerManager();

	void addController(Controller* ctrl);
	void freezeLowerPriorities(int bound);
	void wakeControllers();

	void disableControllers();
	void enableControllers();

protected:
	bool bDisableControllers;

	std::vector <bool>        bset;
	std::list   <Controller*> controllerList;

private:
	void manage(float delta);

	friend class GodClass;

	friend class Controller;

	friend class Encounter;
	friend class Debugger;
};

#endif // CONTROLLERMANAGER_H
