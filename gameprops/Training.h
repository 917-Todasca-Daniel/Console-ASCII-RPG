#pragma once

#include "../Characters.h"


struct TrainingProfile : public CombatProfile {
public:
	TrainingProfile(std::string name);
	virtual ~TrainingProfile() {};
};

class TrainingDummy : public Character {
public:
	TrainingDummy(std::string name);
	virtual void requestAnimation(std::string request, std::string next = "") override;
protected:
	TrainingDummy(std::string name, TrainingProfile* profile);
};

class InstaWin : public Spell {
public:
	InstaWin();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(0AP; win the game)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};

class InstaLose : public Spell {
public:	
	InstaLose();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(0AP; lose the game)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};