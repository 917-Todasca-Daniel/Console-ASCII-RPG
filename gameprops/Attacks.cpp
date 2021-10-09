#include "Attacks.h"

#include "../Encounter.h"
#include "../Behaviours.h"


DaggerAttack::DaggerAttack() : Attack(ALL_STATES, RED, 1, false, "Merc Slash",
	" Merc Slash\n\n    Strike true. A job well done comes with its own satisfactions.\n",
	"    Deals 100 damage to your target. If you kill your target with this attack, gain 6AP, 4MP, and reset all your skills' cooldowns. ") {
	addDescription("On Kill Rewards", "6AP, 4MP, cooldowns reset");
	addDescription("Range", "melee");

	addRequirements(new APCostRequirement(3));
	setTilesOffset(1, rangeTilesOffset);

	addRequirementsDescription();
}
void DaggerAttack::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	std::vector<CombatProfile*> profiles;
	profiles.push_back(source->attackTarget);
	triggerEvents(profiles);
}
void DaggerAttack::triggerEvents(std::vector<CombatProfile*> profiles) {
	Attack::triggerEvents(profiles);
	CombatProfile* target = profiles[0];
	FixedDamageEvent* damageEvent;

	if (!ENCOUNTER->bSimMode) new KillRewardTrigger(profiles[0], source);

	damageEvent = new FixedDamageEvent(100);
	damageEvent->src = source;
	damageEvent->dest = target;
	source->attackTarget = target;
	source->ch->requestAnimation("attack", "idle");
	ENCOUNTER->addEventOnStack(damageEvent);
}

PreciseAttack::PreciseAttack() : Attack(ALL_STATES, RED, 1, false, "Precise Strike",
	" Precise Strike\n\n    A cat uses its precision and style to attack. And once a cat, always a cat.\n",
	"    Deal 150 damage to your target. Can only attack targets on a 5-tiles distance. ") {
	addRequirements(new APCostRequirement(3));
	setPerimeterOffset(5, rangeTilesOffset);

	addRequirementsDescription();
}
void PreciseAttack::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	std::vector<CombatProfile*> profiles;
	profiles.push_back(source->attackTarget);
	triggerEvents(profiles);
}
void PreciseAttack::triggerEvents(std::vector<CombatProfile*> profiles) {
	Attack::triggerEvents(profiles);
	FixedDamageEvent* damageEvent;
	damageEvent = new FixedDamageEvent(100);
	damageEvent->src = source;
	damageEvent->dest = profiles[0];
	ENCOUNTER->addEventOnStack(damageEvent);
}

ArrowAttack::ArrowAttack() : Attack(ALL_STATES, RED, 1, false, "Arrow Attack",
	" Arrow Attack\n\n    What a great day for aiming practice!!\n",
	"    Deal 100 damage to your target in horizontal range.") {
	addRequirements(new APCostRequirement(3));
	setLineTilesOffset(20, rangeTilesOffset, 2);

	addRequirementsDescription();
}
void ArrowAttack::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	std::vector<CombatProfile*> profiles;
	profiles.push_back(source->attackTarget);
	triggerEvents(profiles);
}
void ArrowAttack::triggerEvents(std::vector<CombatProfile*> profiles) {
	Attack::triggerEvents(profiles);
	ScreenObject* obj = new ScreenObject(source->ch->getPosition().first + 8, source->ch->getPosition().second - 2, 50, "arrow");
	obj->show();
	if (profiles[0]->x() < source->x()) obj->flipX(4);
	new ObjectPositionXTransform <ScreenObject>(obj, 10.0f, 10.0f, profiles[0]->ch->getPosition().first, true);

	FixedDamageEvent* damageEvent;
	damageEvent = new FixedDamageEvent(100);
	damageEvent->src = source;
	damageEvent->dest = profiles[0];
	ENCOUNTER->addEventOnStack(damageEvent);
}

Claw::Claw() : Attack(ALL_STATES, RED, 1, false, "Claw",
	" Claw\n\n    Devils know to hit where it hurts most!\n",
	"    Deal 1-hit 100% damage to your target. ") {
	addRequirements(new APCostRequirement(0));
	setTilesOffset(1, rangeTilesOffset);
	addRequirementsDescription();
}
void Claw::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	std::vector<CombatProfile*> profiles;
	profiles.push_back(source->attackTarget);
	triggerEvents(profiles);
}
void Claw::triggerEvents(std::vector<CombatProfile*> profiles) {
	Attack::triggerEvents(profiles);
	FixedDamageEvent* damageEvent;
	damageEvent = new FixedDamageEvent(99999999);
	damageEvent->src = source;
	damageEvent->dest = profiles[0];
	ENCOUNTER->addEventOnStack(damageEvent);
}