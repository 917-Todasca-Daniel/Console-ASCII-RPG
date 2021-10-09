#pragma once

#include "../Characters.h"


class SelfSacrifice : public Spell {
public:
	SelfSacrifice();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(6AP next turn and 15% HP now; gain 6AP)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
class ShadowTeleport : public Spell {
public:
	ShadowTeleport();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(no cost; teleport to enemy)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
class ShadowSwap : public Spell {
public:
	ShadowSwap();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(no cost; swap positions with unit)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);

	bool checkTargetability(CombatProfile* src, CombatProfile* other) override { return true; }
};
class Dash : public Spell {
public:
	Dash();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(2MP; move and 100-damage attack)"; }
	virtual void triggerEvents(std::vector <std::pair <int, int>> profiles);
};
class Hate : public Spell {
public:
	Hate();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; damage equal to missing HP)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
class Frenzy : public Spell {
public:
	Frenzy();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; attack unit 3 times)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};

class ShootingSignal : public Spell {
public:
	ShootingSignal();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; 100 damage to nearby ally enemies)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
	virtual bool checkTargetability(CombatProfile* src, CombatProfile* other) override {
		return other->sentience == src->sentience;
	}
};
class ArrowStorm : public Spell {
public:
	ArrowStorm();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; 100 damage to all enemies in range)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
class Zap : public Spell {
public:
	Zap();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(2AP; 100 damage to units on line)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
class Prayer : public Spell {
public:
	Prayer();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; delayed ally buff)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);

	virtual bool checkTargetability(CombatProfile* src, CombatProfile* candidate) override {
		return src->sentience == candidate->sentience;
	}
};
class Bless : public Spell {
public:
	Bless();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; creates healing zone on tile)"; }
	virtual void triggerEvents(std::vector <std::pair <int, int>> profiles);
};
class ThrowShield : public Spell {
public:
	ThrowShield();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; sacrifice spell for allied DR)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);

	virtual bool checkTargetability(CombatProfile* src, CombatProfile* candidate) override {
		return src->sentience == candidate->sentience;
	}
};
class Stomp : public Spell {
public:
	Stomp();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; 100 / vulnerability 2x damage to enemies on range)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
class Forge : public Spell {
public:
	Forge();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(2AP; create stone pillar)"; }
	virtual void triggerEvents(std::vector <std::pair <int, int>> profiles);
};
class Paz : public Spell {
public:
	Paz();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(2AP; 100 damage to units on same column)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
class HolyHandGrenade : public Spell {
public:
	HolyHandGrenade();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; 100 ranged damage)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
};
class HealingFountain : public Spell {
public:
	HealingFountain(int value);

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; multi-target heal " + intToStr(value) + "%)"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);

	virtual bool checkTargetability(CombatProfile* src, CombatProfile* candidate) override {
		return src->sentience == candidate->sentience;
	}

private:
	int value;
};
class Encourage : public Spell {
public:
	Encourage();

	virtual void castMove(CombatProfile* profile);
	virtual std::string shortDescription() { return "(3AP; buffs allies with AP and MP"; }
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);

	virtual bool checkTargetability(CombatProfile* src, CombatProfile* candidate) override {
		return src->sentience == candidate->sentience;
	}

private:
	int value;
};


