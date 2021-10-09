#include "PlayerCharacters.h"

#include "../engine/Animator.h"
#include "../engine/AnimatorTable.h"

#include "Attacks.h"
#include "Skills.h"


BanditProfile::BanditProfile(std::string name) : CombatProfile(name, AI, 6, 6, 6, 80, 80, 8, 20, 0) {
	addAttack(new DaggerAttack());
}

MuteProfile::MuteProfile(std::string name) : CombatProfile(name, PLAYER, 15, 6, 6, 8, 800, 8, 200, 0) {
	addAttack(new DaggerAttack());
	addSpell(new SelfSacrifice());
	addSpell(new ShadowTeleport());
	addSpell(new ShadowSwap());
	addSpell(new Dash());
	addSpell(new Frenzy());
	addSpell(new Hate());
}

PaladinProfile::PaladinProfile(std::string name) : CombatProfile(name, PLAYER, 6, 6, 6, 200, 200, 200, 200, 80) {
	addAttack(new DaggerAttack());
	addSpell(new Bless());
	addSpell(new Prayer());
	addSpell(new Stomp());
	addSpell(new HolyHandGrenade());
	addSpell(new ThrowShield());
}

GolemCatProfile::GolemCatProfile(std::string name) : CombatProfile(name, PLAYER, 6, 6, 6, 200, 200, 200, 200, 80) {
	addAttack(new PreciseAttack());
	addSpell(new Zap());
	addSpell(new Paz());
	addSpell(new Forge());
}

ArcherProfile::ArcherProfile(std::string name) : CombatProfile(name, PLAYER, 6, 6, 6, 200, 200, 200, 200, 80) {
	addAttack(new ArrowAttack());
	addSpell(new Encourage());
	addSpell(new HealingFountain(35));
	addSpell(new ArrowStorm());
	addSpell(new ShootingSignal());
}


//	CHARACTERS
TheMute::TheMute() : Character("The Mute Bard") {
	setCombatProfile(new MuteProfile("The Mute"));
	openTable("themute");
	requestAnimation("idle");
	show();
	setName("mute");
	for (auto it : parts)
		it.first->setName("muteAnim");
}
void TheMute::requestAnimation(std::string name, std::string nextAnim) {
	Character::requestAnimation(name, nextAnim);
}

GolemCat::GolemCat(std::string name) : Character(name) {
	setCombatProfile(new GolemCatProfile("Golem Cat"));
	openTable("themute");
	requestAnimation("idle");
	show();
	setName("mute");
	for (auto it : parts)
		it.first->setName("muteAnim");
	for (auto& it : speakMessages) {
		it.clear();
		it.push_back("Meow!");
		it.push_back("Meoow...");
		it.push_back("Meow, meo-meow!");
	}
}
void GolemCat::requestAnimation(std::string name, std::string nextAnim) {
	Character::requestAnimation(name, nextAnim);
}

CHPaladin::CHPaladin() : Character("Paladin") {
	setCombatProfile(new PaladinProfile("Paladin"));
	openTable("paladin");
	requestAnimation("idle");
	show();
}
void CHPaladin::requestAnimation(std::string name, std::string nextAnim) {
	Character::requestAnimation(name);
}

Archer::Archer(std::string name) : Character(name) {
	setCombatProfile(new ArcherProfile(name));
	openTable("archer");
	requestAnimation("idle");
	show();
	setName("archer");
	for (auto it : parts)
		it.first->setName("archerAnim");
}

void Archer::requestAnimation(std::string request, std::string next) {
	if (request == "walk1") {
		Character::requestAnimation("walk0", "");
		for (auto it : parts)
			it.first->flipX(flipAxis);
	}
	else {
		Character::requestAnimation(request, next);
		if (request == "cast") {
			for (auto it : parts)
				it.first->setStartFrame(3);
		}
	}
}
