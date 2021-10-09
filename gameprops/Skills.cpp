#include "Skills.h"

#include "../Encounter.h"
#include "../engine/Animator.h"
#include "../engine/AnimatorTable.h"
#include "../Behaviours.h"


SelfSacrifice::SelfSacrifice() : Spell(ALL_STATES, RED, 0, true, "Self Sacrifice",
	" Self Sacrifice\n\n    '(...) The story of this scar? I was running away from a murder scene, when I got ambushed by about a dozen of armed men. No, they did not lay any of their weapons on me. It's just that I was exhausted, panting as I tried to catch my breath, and I needed a little push to make my body listen to my will again.'\n",
	"    Deal 15% of your max HP to yourself. Gain 6AP this turn. You will have 6AP less regen next turn.") {
	addDescription("HP Cost", "15% HP");

	addRequirements(new Cooldown(1, 1));
	addRequirementsDescription();
}
void SelfSacrifice::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	std::vector<CombatProfile*> profiles;
	if (!ENCOUNTER->bSimMode)
		triggerEvents(profiles);
}
void SelfSacrifice::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	source->ch->requestAnimation("sacrifice", "idle");
	TrueScallingMAXHPDamageEvent* damageEvent = new TrueScallingMAXHPDamageEvent(20);
	damageEvent->src = source;
	damageEvent->dest = source;
	source->AP += 6;
	source->fatigue += 6;
	ENCOUNTER->addEventOnStack(damageEvent);
}

ShadowTeleport::ShadowTeleport() : Spell(ALL_STATES, RED, 0, true, "Shadow Teleport",
	" Shadow Teleport\n\n    There's always a shadow in your back. But whose is that right now?\n",
	"    Choose any other unit on the battlefield and teleport to its VULNERABILITY tile. If that tile is unavailable, teleport to any random adjacent tile. If every adjacent tile is unavailable, you cannot cast this spell. ") {
	addDescription("VULNERABILITY tile", vulnerable_description);
	addDescription("Cast Range", "global");

	addRequirements(new Cooldown(3, 3));

	addRequirementsDescription();
}
void ShadowTeleport::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void ShadowTeleport::triggerEvents(std::vector<CombatProfile*> profiles) {
	CombatProfile* target = profiles[0];
	std::pair <int, int> p(-1, -1);
	if (ENCOUNTER->checkValability(target->getVulnerableTile())) {
		p = target->getVulnerableTile();
	}
	else {
		std::pair <int, int> dxy[] = { {-1, 0}, {1, 0}, {0, 1}, {0, -1} };
		random_shuffle(dxy, dxy + 4);
		for (int i = 0; i < 4; ++i) {
			std::pair <int, int> x = { dxy[i].first + target->x(), dxy[i].second + target->y() };
			if (ENCOUNTER->checkValability(x)) p = x, i = 5;
		}
	}

	if (p.first == -1) {
		ENCOUNTER->addErrorMessage("Cannot teleport, no available adjacent tiles!");
	}
	else {
		if (ENCOUNTER->bSimMode == false) {
			Animator* anim = new Animator("teleportpart", source->ch->getPosition().first, source->ch->getPosition().second, 0, true, true);
			anim->setName("tpAnim");
		}
		source->ch->requestAnimation("kneelup", "idle");
		Spell::triggerEvents(profiles);
		TeleportEvent* event = new TeleportEvent(p);
		event->src = source;
		ENCOUNTER->addEventOnStack(event);
	}
}

ShadowSwap::ShadowSwap() : Spell(ALL_STATES, RED, 0, true, "Shadow Swap",
	" Shadow Swap\n\n    '(...) That merc is a devil, I swear. I went ahead to strike down the damned fool down when I had the chance. But in the blink of an eye, he was gone, nowhere to be found! And instead, one of us laid on the ground, his blood on my dagger.'\n",
	"    Choose any other unit on the battlefield and swap positions with it.") {
	addDescription("Cast Range", "global");

	addRequirements(new Cooldown(3, 3));
	addRequirementsDescription();
}
void ShadowSwap::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void ShadowSwap::triggerEvents(std::vector<CombatProfile*> profiles) {
	CombatProfile* target = profiles[0];

	auto srcTile = source->getTile();
	auto targetTile = target->getTile();

	if (ENCOUNTER->bSimMode == false) {
		Animator* anim = new Animator("teleportpart", source->ch->getPosition().first, source->ch->getPosition().second, 0, true, true);
		anim->setName("tpAnim");
	}
	source->ch->requestAnimation("kneelup", "idle");
	Spell::triggerEvents(profiles);

	TeleportEvent* event = new TeleportEvent(targetTile);
	event->src = source;
	ENCOUNTER->addEventOnStack(event);

	TeleportEvent* swap_event = new TeleportEvent(srcTile);
	swap_event->src = target;
	ENCOUNTER->addEventOnStack(swap_event);

	target->ch->hideVisual(0.8f);
}

Dash::Dash() : Spell(ALL_STATES, RED, 0, false, "Dash",
	" Dash\n\n    Striking hard is not all that matters. Do not forget to be quick on your feet and strike fast.\n",
	"    Select an available tile in range and move to it. If that tile is the VULNERABILITY tile of any other units, deal 100 damage to those units.") {
	addDescription("VULNERABILITY tile", vulnerable_description);
	addDescription("Range", "line 5");

	addRequirements(new MPCostRequirement(2), new Cooldown(1, 1));
	setLineTilesOffset(5, rangeTilesOffset);

	addRequirementsDescription();
}
void Dash::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->requestLineAttackTileSelection(this, 1, 5, source->getTile().first, source->getTile().second, true);
}
void Dash::triggerEvents(std::vector<std::pair <int, int>> tiles) {
	Spell::triggerEvents(tiles);
	auto tile = tiles[0];

	for (auto& it : ENCOUNTER->getCharacters()) {
		if (it->cp == source) continue;
		if (it->cp->getVulnerableTile() == tile) {
			DamageEvent* dmg = new FixedDamageEvent(100);
			dmg->src = source;
			dmg->dest = it->cp;
			ENCOUNTER->addEventOnStack(dmg);
			VulnerabilityConsumptionEvent* other = new VulnerabilityConsumptionEvent();
			other->dest = it->cp;
			ENCOUNTER->addEventOnStack(other);
		}
	}

	source->setTile(tile);
}

Hate::Hate() : Spell(ALL_STATES, RED, 0, true, "Hate",
	" Hate\n\n    '(...) You are fueled by hate, mercenary. And the worst thing is that you don't seem to realize it.'\n",
	"    Deal as much damage as your missing HP to any other chosen unit. Cannot cast unless you're standing on the VULNERABILITY tile of your target. Must wait 5 turns from the start of the fight to use this skill.") {
	addDescription("VULNERABILITY tile", vulnerable_description);
	addDescription("Range", "melee");

	addRequirements(new Cooldown(5, 0), new APCostRequirement(3));
	setTilesOffset(1, rangeTilesOffset);

	addRequirementsDescription();
}
void Hate::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void Hate::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	CombatProfile* target = profiles[0];
	FixedDamageEvent* damageEvent;
	int value = source->init->HP - source->HP;
	damageEvent = new FixedDamageEvent(value);
	damageEvent->src = source;
	damageEvent->dest = target;
	source->attackTarget = target;
	source->ch->requestAnimation("attack", "idle");
	ENCOUNTER->addEventOnStack(damageEvent);
}

Frenzy::Frenzy() : Spell(ALL_STATES, RED, 0, true, "Frenzy",
	" Frenzy\n\n    Sticking to what you know is not that bad.\n",
	"    Choose any other unit. Use your attack on your target 3 times, on a lesser cost.") {
	addDescription("Range", "melee");

	addRequirements(new Cooldown(2, 2), new APCostRequirement(3));
	setTilesOffset(1, rangeTilesOffset);

	addRequirementsDescription();
}
void Frenzy::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void Frenzy::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	for (int i = 0; i < 3; ++i) {
		CombatProfile* target = profiles[0];
		FixedDamageEvent* damageEvent;

		if (!ENCOUNTER->bSimMode) new KillRewardTrigger(profiles[0], source);

		damageEvent = new FixedDamageEvent(100);
		damageEvent->src = source;
		damageEvent->dest = target;
		source->attackTarget = target;
		ENCOUNTER->addEventOnStack(damageEvent);
	}
	source->ch->requestAnimation("attack", "idle");
}

HealingFountain::HealingFountain(int value) : Spell(ALL_STATES, DCYAN, 1, false, "Heal Fountain",
	" Healing Fountain\n\n    '(...) I have heard you saying you would die for my blessing and for your honor. I thank you. But when I am here, it is not your time to die.'\n",
	"    Heal an ally target and all adjacent units for " + intToStr(value) + " HP. Must wait 3 turns from the start of the fight to use this skill.") {
	this->value = value;
	addDescription("Range", "5 perimeter");

	addRequirements(new APCostRequirement(3), new Cooldown(5, 2));
	setTilesOffset(1, effectTilesOffset);
	setPerimeterOffset(5, rangeTilesOffset);

	addRequirementsDescription();
}
void HealingFountain::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void HealingFountain::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	for (auto it : Encounter::getInstance()->getProfiles()) {
		int dist = abs(it->x() - profiles[0]->x()) + abs(it->y() - profiles[0]->y());
		if (dist == 1 && it->sentience == source->sentience) profiles.push_back(it);
	}

	for (auto it : profiles) {
		HealEvent* event = new HealEvent(value);
		event->src = source;
		event->dest = it;
		source->ch->requestAnimation("heal", "idle");
		ENCOUNTER->addEventOnStack(event);
	}
}

Encourage::Encourage() : Spell(ALL_STATES, DCYAN, 0, false, "Encourage",
	" Encourage\n\n    Who said charisma doesn't win a war?\n",
	"    Encourage all allied units in range, increasing their MP by 4 and their AP by 3, carried above their threshold.") {
	this->value = value;
	addDescription("Range", "5");

	addRequirements(new APCostRequirement(3), new Cooldown(5, 5));
	setTilesOffset(5, rangeTilesOffset);

	addRequirementsDescription();
}
void Encourage::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	std::vector <CombatProfile*> profiles;
	triggerEvents(profiles);
}
void Encourage::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	for (auto it : Encounter::getInstance()->getProfiles()) {
		int dist = abs(it->x() - source->x()) + abs(it->y() - source->y());
		if (dist <= 5 && it->sentience == source->sentience) profiles.push_back(it);
	}

	for (auto it : profiles) {
		EncourageEvent* event = new EncourageEvent();
		event->src = source;
		event->dest = it;
		ENCOUNTER->addEventOnStack(event);
	}
}

ArrowStorm::ArrowStorm() : Spell(ALL_STATES, RED, 0, false, "Arrow Storm",
	" Arrow Storm\n\n    Where did all these arrows come from?\n",
	"    Chose an unit from battlefield in range. Deal 100 damage to it and any other units on the same side of the battlefield that are in range of the skill. ") {

	addRequirements(new APCostRequirement(3), new Cooldown(1, 1));
	setTilesOffset(0, effectTilesOffset);
	setChessOffset(15, 1, 0, rangeTilesOffset);

	addRequirementsDescription();
}
void ArrowStorm::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void ArrowStorm::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	auto target = profiles[0];
	int parity = (target->x() + target->y()) % 2;
	int sgn = target->x() - source->x();
	for (auto ch : ENCOUNTER->getCharacters()) {
		if (sgn * (ch->cp->x() - source->x()) > 0) {
			if (parity == (ch->cp->x() + ch->cp->y()) % 2) {
				DamageEvent* dmg = new FixedDamageEvent(100);
				dmg->src = source;
				dmg->dest = ch->cp;
				ENCOUNTER->addEventOnStack(dmg);
			}
		}
	}
}

ShootingSignal::ShootingSignal() : Spell(ALL_STATES, RED, 0, true, "Air Support",
	" Air Support\n\n    Did anyone call for back-up!\n",
	"    Choose an ally. Damage all its adjacent enemies for 100 HP.") {
	addRequirements(new APCostRequirement(3), new Cooldown(1, 1));
	setTilesOffset(1, effectTilesOffset);
	setTilesOffset(8, rangeTilesOffset);

	addRequirementsDescription();
}
void ShootingSignal::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void ShootingSignal::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	auto ally = profiles[0];
	std::vector <CombatProfile*> targets;
	for (auto it : Encounter::getInstance()->getProfiles()) {
		if (it->sentience != source->sentience) {
			int dist = abs(ally->x() - it->x()) + abs(ally->y() - it->y());
			if (dist <= 1) targets.push_back(it);
		}
	}

	for (auto it : targets) {
		DamageEvent* event = new FixedDamageEvent(500);
		event->src = source;
		event->dest = it;
		ENCOUNTER->addEventOnStack(event);
	}
}

Zap::Zap() : Spell(ALL_STATES, RED, 1, true, "Zap",
	" Zap\n\n    '(...) Eenie meenie miney mo, you are toast and need to go!'\n",
	"    Select any other unit on the battlefield. Deal 150 damage to it and all other targets on the same line.") {
	addDescription("Range", "global");

	addRequirements(new APCostRequirement(2), new Cooldown(0, 0));

	addRequirementsDescription();
}
void Zap::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void Zap::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	for (auto it : Encounter::getInstance()->getProfiles()) {
		if (it->y() == profiles[0]->y() && it != profiles[0]) profiles.push_back(it);
	}

	for (auto it : profiles) {
		DamageEvent* event = new FixedDamageEvent(150);
		event->src = source;
		event->dest = it;
		ENCOUNTER->addEventOnStack(event);
	}
}

Paz::Paz() : Spell(ALL_STATES, RED, 1, true, "Paz",
	" Paz\n\n    '(...) !og ot deen dna tsaot era uoy ,om yenim eineem eineE'\n",
	"    Select any other unit on the battlefield. Deal 150 damage to it and all other targets on the same column.") {
	addDescription("Range", "global");

	addRequirements(new APCostRequirement(2), new Cooldown(0, 0));

	addRequirementsDescription();
}
void Paz::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void Paz::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	for (auto it : Encounter::getInstance()->getProfiles()) {
		if (it->x() == profiles[0]->x() && it != profiles[0]) profiles.push_back(it);
	}

	for (auto it : profiles) {
		DamageEvent* event = new FixedDamageEvent(150);
		event->src = source;
		event->dest = it;
		ENCOUNTER->addEventOnStack(event);
	}
}

Forge::Forge() : Spell(ALL_STATES, RED, 0, true, "Forge",
	" Forge\n\n    Like playing with the sand in a litter box!\n",
	"    Select any available tile. Create a stone pillar object on it.") {
	addDescription("Range", "global");

	addRequirements(new APCostRequirement(2), new Cooldown(0, 0));
	setTilesOffset(5, rangeTilesOffset);

	addRequirementsDescription();
}
void Forge::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->requestAttackTileSelection(this, 1, 30, source->getTile().first, source->getTile().second, true);
}
void Forge::triggerEvents(std::vector<std::pair <int, int>> tiles) {
	Spell::triggerEvents(tiles);
	auto tile = tiles[0];

	auto pillar = new StonePillar("pillar");
	pillar->cp->setTile(tile);

	ENCOUNTER->addCharacter(pillar);
}

Bless::Bless() : Spell(ALL_STATES, DCYAN, 0, false, "Bless",
	" Bless\n\n    '(...) The Divine accepts all, and gives you all. Just stand in the light, and let it invigorate you.'\n",
	"    Bless a tile in range. At the beginning of each next 5 turns, the unit standing on it will get healed for 100 HP. If you Bless a tile that is already affected by Blesh, refresh its duration.") {
	addDescription("Range", "5");

	addRequirements(new APCostRequirement(3), new Cooldown(0, 0));
	setTilesOffset(5, rangeTilesOffset);

	addRequirementsDescription();
}
void Bless::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->requestAttackTileSelection(this, 1, 5, source->getTile().first, source->getTile().second, true, false);
}
void Bless::triggerEvents(std::vector<std::pair <int, int>> tiles) {
	Spell::triggerEvents(tiles);
	auto tile = tiles[0];

	auto triggers = ENCOUNTER->triggers;
	for (auto& trigger : triggers) {
		TileBlessTrigger* cast = dynamic_cast <TileBlessTrigger*> (trigger);
		if (cast && cast->tile == tile) {
			cast->duration = 5;
			return;
		}
	}

	new TileBlessTrigger(5, tile);
}

Prayer::Prayer() : Spell(ALL_STATES, DCYAN, 0, true, "Prayer",
	" Prayer\n\n    '(...) '\n",
	"    Select an allied unit. When the turn ends, they will gain a buff.") {
	addDescription("Range", "5");

	addRequirements(new APCostRequirement(3), new Cooldown(1, 1));
	setTilesOffset(5, rangeTilesOffset);

	addRequirementsDescription();
}
void Prayer::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void Prayer::triggerEvents(std::vector <CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	auto profile = profiles[0];

	new PrayerEvent(profile);
}

ThrowShield::ThrowShield() : Spell(ALL_STATES, DCYAN, 1, false, "Throw Shield",
	" Throw Shield\n\n    Sharing is caring!\n",
	"    Select an allied unit to throw your shield at. You will lose this spell and 20% DR. They will gain this spell instead and receive 20% DR.") {
	addDescription("Range", "8");

	addRequirements(new APCostRequirement(3), new Cooldown(0, 0));
	setTilesOffset(8, rangeTilesOffset);

	addRequirementsDescription();
}
void ThrowShield::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void ThrowShield::triggerEvents(std::vector <CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);
	auto profile = profiles[0];

	profile->spellArr.push_back(this);
	auto sp = source->spellArr;
	profile->ch->buff("+20 DR!", YELLOW);
	source->ch->buff("-20 DR!", RED);
	profile->DR += 20;
	source->DR -= 20;
	sp.erase(std::remove(sp.begin(), sp.end(), this), sp.end());
}

Stomp::Stomp() : Spell(ALL_STATES, RED, 0, false, "Stomp",
	" Stomp\n\n    Stomp description!\n",
	"    Deal 100 damage to all enemies in range. For every VULNERABILITY tile you stand on, deal double the damage and heal yourself for 50 HP. ") {
	addDescription("VULNERABILITY tile", vulnerable_description);
	addDescription("Range", "2");

	addRequirements(new APCostRequirement(3), new Cooldown(2, 2));
	setTilesOffset(5, rangeTilesOffset);

	addRequirementsDescription();
}
void Stomp::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	triggerEvents(std::vector <CombatProfile*>());
}
void Stomp::triggerEvents(std::vector <CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);

	int vulnerables = 0;
	std::vector <CombatProfile*> targets;
	for (auto cp : ENCOUNTER->getProfiles()) {
		if (cp->sentience == source->sentience) continue;
		int dist = abs(source->x() - cp->x()) + abs(source->y() - cp->y());
		if (dist <= 2) {
			targets.push_back(cp);
		}
		if (cp->getVulnerableTile() == std::pair <int, int>(source->x(), source->y())) {
			vulnerables++;
			VulnerabilityConsumptionEvent* other = new VulnerabilityConsumptionEvent();
			other->dest = cp;
			ENCOUNTER->addEventOnStack(other);
		}
	}

	int dmg = 100;
	int heal = 20 * vulnerables;
	for (int i = 0; i < vulnerables; ++i) dmg *= 2;

	if (heal > 0) {
		HealEvent* event = new HealEvent(heal);
		event->dest = source;
		ENCOUNTER->addEventOnStack(event);
	}

	for (auto& target : targets) {
		DamageEvent* dmg = new FixedDamageEvent(100);
		dmg->src = source;
		dmg->dest = target;
		ENCOUNTER->addEventOnStack(dmg);
	}
}

HolyHandGrenade::HolyHandGrenade() : Spell(ALL_STATES, RED, 1, false, "Holy Hand Grenade",
	" Holy Hand Grenade\n\n    Boom!\n",
	"    Select any other unit in range. Deal 100 damage to it.") {
	addRequirements(new APCostRequirement(3), new Cooldown(1, 1));
	setTilesOffset(8, rangeTilesOffset);

	addRequirementsDescription();
}
void HolyHandGrenade::castMove(CombatProfile* profile) {
	CombatMoveInterface::castMove(profile);
	ENCOUNTER->combatMoveTargetRequestSetup(this, 1);
}
void HolyHandGrenade::triggerEvents(std::vector<CombatProfile*> profiles) {
	Spell::triggerEvents(profiles);

	for (auto it : profiles) {
		DamageEvent* event = new FixedDamageEvent(100);
		event->src = source;
		event->dest = it;
		ENCOUNTER->addEventOnStack(event);
	}
}
