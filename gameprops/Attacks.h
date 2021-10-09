#pragma once

#include "../Characters.h"


class DaggerAttack : public Attack {
public:
	DaggerAttack();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; 100 damage)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};

class PreciseAttack : public Attack {
public:
	PreciseAttack();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; 150 damage)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};

class Claw : public Attack {
public:
	Claw();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(0; 99999999 damage)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};

class ArrowAttack : public Attack {
public:
	ArrowAttack();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; 100 damage)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
