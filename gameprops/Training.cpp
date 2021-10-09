#include "Training.h"

#include "../Encounter.h"
#include "../engine/Animator.h"
#include "../engine/AnimatorTable.h"

#include "Attacks.h"
#include "Skills.h"


TrainingProfile::TrainingProfile(std::string name) : CombatProfile(name, PLAYER, 0, 0, 0, 0, 0, 0, 1000, 0) {
	attackArr.push_back(new Claw());
	spellArr.push_back(new InstaWin());
	spellArr.push_back(new InstaLose());
	attackRange = 1;
}
TrainingDummy::TrainingDummy(std::string name) : TrainingDummy(name, new TrainingProfile(name)) {
}
void TrainingDummy::requestAnimation(std::string request, std::string next) {
	if (request == "walk0") {
		Character::requestAnimation("walk1", "");
		for (auto it : parts)
			it.first->flipX(flipAxis);
	}
	else {
		Character::requestAnimation(request, next);
	}
}

TrainingDummy::TrainingDummy(std::string name, TrainingProfile* profile) {
	setCombatProfile(profile);
	openTable("dummytable");
	requestAnimation("idle");
	show();
	setName("imp");
	for (auto it : parts)
		it.first->setName("impAnim");
}

InstaWin::InstaWin() : Spell(ALL_STATES, RED, 0, true, "Win the Game", " Win the game\n\n\n", "    Costs nothing. Win the game.") { }
void InstaWin::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	triggerEvents(std::vector <CombatProfile*>());
}
void InstaWin::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	auto cast = dynamic_cast <TrainingFight*> (ENCOUNTER);
	if (cast != nullptr) {
		cast->bWin = true;
	}
}

InstaLose::InstaLose() : Spell(ALL_STATES, RED, 0, true, "Lose the Game", " Lose the game\n\n\n", "    Costs nothing. Lose the game.") { }
void InstaLose::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	triggerEvents(std::vector <CombatProfile*>());
}
void InstaLose::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	auto cast = dynamic_cast <TrainingFight*> (ENCOUNTER);
	if (cast != nullptr) {
		cast->bLose = true;
	}
}
