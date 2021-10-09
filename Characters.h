#pragma once

#include "engine/PrimitiveObject.h"

#include "engine/stdc++.h"

#define	POINT	std::pair <int, int>

enum SENTIENCE			{ PLAYER = 0, OBJECT, AI, SENTIENCE_COUNT };
enum TARGET_ALIVESTATE	{ ALL_STATES = 0, ONLY_ALIVE, ONLY_DEAD };

class Character;	
class CombatMoveInterface;
class Spell;
class Attack;
class PrimitiveAIBrain;
struct CombatProfile;
template <typename type>
class CombatGridLineTraceBehaviour;


//	COMBAT PROFILE
struct CombatProfile {
	CombatProfile(std::string name = "", SENTIENCE sentience = OBJECT, int AP = 0, int startAP = 0, int regenAP = 0, int MP = 0, int startMP = 0, int regenMP = 0, int HP = 0, int DR = 0);
	virtual ~CombatProfile();

	//	PUBLIC VARIABLES
	Character*				ch = nullptr;
	CombatProfile*			init = nullptr;
	CombatProfile*			attackTarget = nullptr;
	PrimitiveAIBrain*		brainAI = nullptr;
	CombatMoveInterface*	lastMove = nullptr;

	std::string	name = "";
	SENTIENCE	sentience = PLAYER;
	int			HP = 0, MP = 0, startMP = 0, regenMP = 0, AP = 0, startAP = 0, regenAP = 0, DR = 0;
	int			fatigue = 0, fatigueMP = 0, attackRange = 1;
	bool		bDead = false, bFirstTurn = true;

	//	UTILITY FUNCTIONS
	inline std::string hpString() { return intToStr(HP) + "/" + intToStr(init->HP) + " HP"; }
	inline std::string apString() { return intToStr(AP) + "/" + intToStr(init->AP) + " AP"; }
	inline std::string mpString() { return intToStr(MP) + "/" + intToStr(init->MP) + " MP"; }

	inline POINT getTile()				{ return { grid_x, grid_y }; }
	inline POINT getVulnerableTile()	{ return { grid_x + vulnerable_x, grid_y + vulnerable_y }; }
	inline int	 x()					{ return grid_x; }
	inline int	 y()					{ return grid_y; }

	inline const std::vector<Spell*>	getSpells	() { return spellArr; }
	inline const std::vector<Attack*>	getAttacks	() { return attackArr; }

	inline bool checkAttackFlip() { return (attackTarget->grid_x < grid_x); }

	inline void addSpell	(Spell*	spell)		{ spellArr.push_back(spell); }
	inline void addAttack	(Attack* attack)	{ attackArr.push_back(attack); }

	inline void setTile				(POINT tile) { this->grid_x = tile.first, this->grid_y = tile.second; }
	inline void setVulnerableTile	(POINT tile) { this->vulnerable_x = tile.first, this->vulnerable_y = tile.second; }

	//	COMBAT FUNCTIONS
	virtual void applyMovementCost(int value);
	virtual void turnBeginEffects();
	virtual void resetCooldowns();

protected:
	std::vector <Attack*>	attackArr;
	std::vector <Spell*>	spellArr;

	int grid_x = 0, grid_y = 0;
	int vulnerable_x = 0, vulnerable_y = 0;

	struct animationFlags {
		bool bMoving;
		int  movementDirection;
	}	prev, pres;

	friend class Character;
	friend class ThrowShield;
	friend class Encounter;

	friend class ForestBanditsFight;
	friend class DiningFight;
	friend class TrainingFight;
};

struct ObjectProfile : public CombatProfile {
	ObjectProfile(std::string name = "", int HP = 0, int DR = 0) 
		: CombatProfile(name, OBJECT, 0, 0, 0, 0, 0, 0, HP, DR) { }
};

struct StonePillarProfile : public ObjectProfile {
	StonePillarProfile() : ObjectProfile(createName(), 1, 0) {};
	static int counter;
private:
	std::string createName() {
		return "Stone Pillar #" + std::to_string(counter++);
	}
};

class CharacterSpeakLinesTracker : public PrimitiveObject {
public:
	CharacterSpeakLinesTracker(float maximumWait) : maximumWait(maximumWait), wait(0), PrimitiveObject() {}

	bool roll() {
		if (wait >= maximumWait || randomRoll()) {
			wait = 0;
			return true;
		}
		return false;
	}

	void reset() {
		wait = 0;
	}

protected:
	virtual void loop(float delta) override {
		if (bDestroyed) return;
		PrimitiveObject::loop(delta);
		wait += delta;
	}
	bool randomRoll() {
		auto value = randBetween(0, maximumWait * waitScale);
		if (value < wait) return true;
		return false;
	}

	const float waitScale = 4;
	float maximumWait, wait;
};

//	CHARACTER
class Character : public PrimitiveObject {
public:
	Character(std::string name= "");

	enum SPEAKTYPES { CAST = 0, DEALDMG, TAKEDMG, TURNBEGIN, SUPPORT, CRITICAL, HEAL, THANKS, DEATH, SPEAK_CNT };

	virtual void destroy() override;

	virtual void	requestAnimation	(std::string name, std::string nextAnim = "");
	void			flipX				();
	void			overrideRenderPlane	();
	void			resetPriority		();

	void			createAttackVisualEffects	(int dmg = 0);	
	void			buff						(std::string desc, COLOR color);	
	void			speak						(const std::string &msg, COLOR color = WHITE);
	void			speak						(SPEAKTYPES type, COLOR color = WHITE);

	void setPosition	(int x, int y);
	void setColor		(Color color);

	std::pair <int, int> getPosition() { return std::pair <int, int>(x, y); }

	void show		();
	void hide		();
	void showVisual	();
	void hideVisual	();
	void hideVisual	(float duration);

	CombatProfile* cp;

	static void resetTrackers() {
		for (int i = 0; i < SPEAK_CNT; ++i)
			lineTrackers[i]->reset();
	}

protected:
	virtual ~Character();

	virtual void loop(float delta) override;

	bool bHide;
	int  x, y, head_x = 6, head_y = -6;
	int  flipAxis = 15;

	void setCombatProfile(CombatProfile* profile) { cp = profile; profile->ch = this; }

	std::vector <std::pair <class Animator*, class AnimatorTable*>> parts;
	void openTable(std::string name);

	std::vector <std::string> speakMessages[SPEAK_CNT] = {
		{"Focus..."},
		{"Bow to my blade!", "Taste my bits and bytes!"},
		{"I will not concede!", "My brackets are still holding!"},
		{"Admire my true colors!", "Enough idling around!"},
		{"Stay strong!"},
		{"Everything is becoming darker..."},
		{"Be blessed."},
		{"Thank you!"},
		{"My bits remain..."},
	};

private:
	int visualIdx = 0;
	std::vector <std::pair <int, int>> dmgVisualOffsets = { {0, 0}, {-1, -1}, {1, 1}, {10, 2}, {11, 1}, {12, 4}, {1, 5}, {9, 1}, {9, 3}, {-1, 2}, {-2, 0}, {-2, 2}, {-3, 0} };
	class Timer *timer;

	static int chs_count;
	static class CharacterSpeakLinesTracker* lineTrackers[SPEAK_CNT];

	friend class CombatGridLineTraceBehaviour <Character>;
};

class Object : public Character {
public:
	Object(std::string name);
};

class StonePillar : public Object {
public:
	StonePillar(std::string name);
};

//	REQUIREMENTS
class CombatMoveRequirement {
public:
	CombatMoveRequirement() {};
	virtual bool check(CombatProfile* profile) = 0;
	virtual void applyConsequences(CombatProfile *profile) = 0;
	virtual std::pair <std::string, std::string> descriptionGen() = 0;
};

class Cooldown : public CombatMoveRequirement {
public:
	Cooldown(int value, int charged = 0) : value(value), charged(charged-1) {}
	virtual bool check(CombatProfile* profile) {
		return charged >= value;
	}
	virtual void applyConsequences(CombatProfile* profile) {
		charged = 0;
	}	virtual std::pair <std::string, std::string> descriptionGen() { 
		std::string desc = intToStr(value);
		return { "Cooldown", desc };
	}
	int value, charged;
};
class MPCostRequirement : public CombatMoveRequirement {
public:
	MPCostRequirement(int value) : value(value) {}
	virtual bool check(CombatProfile* profile) {
		return (profile->MP >= value);
	}
	virtual void applyConsequences(CombatProfile* profile) {
		profile->MP -= value;
	}	virtual std::pair <std::string, std::string> descriptionGen() { return { "MP cost", intToStr(value) }; }
	int value;
};
class APCostRequirement : public CombatMoveRequirement {
public:
	APCostRequirement(int value) : value(value) {}
	virtual bool check(CombatProfile* profile) {
		return (profile->AP >= value);
	}
	virtual void applyConsequences(CombatProfile* profile) {
		profile->AP -= value;
	}	virtual std::pair <std::string, std::string> descriptionGen() { return { "AP cost", intToStr(value) }; }
	int value;
};

//	COMBAT INTERFACE
enum ValidityCode { VC_OK = 0, VC_NO_TARGETS = 1, VC_COUNT };

class CombatMoveInterface {
public:
	CombatMoveInterface(TARGET_ALIVESTATE targetAliveState, COLOR color, int minTargets, bool bGlobal, std::string name, std::string fantasy, std::string techDescription)
		: targetAliveState(targetAliveState), color(color), bGlobal(bGlobal), minTargets(minTargets), name(name), fantasy(fantasy), techDescription(techDescription) { 
			source = nullptr; 
			maxDescriptionLen = 0; 
	}
	virtual ~CombatMoveInterface()	{ 
		for (auto requirement : requirements) delete requirement; 
	}

	virtual			void castMove		(CombatProfile* profile)					{ }
	virtual			void triggerEvents	(std::vector <CombatProfile*> profiles)		= 0;
	virtual			void triggerEvents	(std::vector <std::pair <int, int>> tiles)	= 0;

	virtual			std::string shortDescription() = 0;

	inline			std::string getName				() { return name; }
	inline			bool		isGlobal			() { return bGlobal; }
					COLOR		getColor			() { return color; }
	virtual inline	std::string descriptionGen		() { return fantasy + '\n' + techDescription + "\n\n" + descriptionHelper(); }

	virtual bool checkRequirements(CombatProfile* profile) {
		source = profile;
		for (auto it : requirements) if (!it->check(profile)) return false;
		return true;
	}

	void resetCooldown() {
		for (auto& it : requirements) {
			Cooldown* req = dynamic_cast <Cooldown*>(it);
			if (req != nullptr) req->charged = req->value;
		}
	}

	virtual ValidityCode						checkMoveValidity	(Character* caster, std::vector <Character*> characters);
	virtual std::vector <CombatProfile*>		targetsInRange		(Character* caster, std::vector <Character*> characters);
	virtual std::vector <std::pair <int, int>>	rangeTiles			(int x, int y) {
		std::vector <std::pair <int, int>> ret;
		for (auto it : rangeTilesOffset) ret.push_back({ x + it.first, y + it.second });
		return ret;
	}
	virtual std::vector <std::pair <int, int>>	effectTiles			(int x, int y) {
		std::vector <std::pair <int, int>> ret;
		for (auto it : effectTilesOffset) ret.push_back({ x + it.first, y + it.second });
		return ret;
	}

	CombatProfile* source;

protected:
	static std::string vulnerable_description;

	virtual bool checkTargetability(CombatProfile* src, CombatProfile* candidate) {
		if (src == candidate) return false;
		if (targetAliveState != ALL_STATES && targetAliveState != candidate->bDead + 1) return false;
		return true;
	}

	TARGET_ALIVESTATE targetAliveState = ONLY_ALIVE;
	COLOR	color; 
	int		minTargets;
	bool	bGlobal;
	std::string name, fantasy, techDescription;
	
	std::vector <Character*> validTargets;
	std::vector <CombatMoveRequirement*> requirements;

	int maxDescriptionLen;
	std::vector <std::string> descriptionLine;
	std::vector <std::pair <int, int>> rangeTilesOffset;
	std::vector <std::pair <int, int>> effectTilesOffset;

	void setTilesOffset(int range, std::vector <std::pair <int, int>>& offset) {
		offset.clear();
		for (int i = -range, j; i <= range; ++i)
			for (j = -(range - (std::max)(i, -i)); j <= range - (std::max)(i, -i); ++j)
				offset.push_back({ j, i });
	}
	void setLineTilesOffset(int range, std::vector <std::pair <int, int>>& offset, int exclude = -1) {
		offset.clear();
		for (int i = -range; i <= range; ++i) {
			if (i <= exclude && i >= -exclude) continue;
			offset.push_back({ i, 0 });
		}
	}
	void setPerimeterOffset(int range, std::vector <std::pair <int, int>>& offset) {
		offset.clear();
		for (int i = -range; i <= range; ++i) {
			int other = range - abs(i);
			if (other != 0) offset.push_back({ i, -other }), offset.push_back({ i, other });
			else offset.push_back({ i, 0 });
		}
	}
	void setChessOffset(int range, int parity, int colExclude, std::vector <std::pair <int, int>>& offset) {
		offset.clear();
		for (int i = -range; i <= range; ++i) {
			if (i == colExclude) continue;
			for (int j = -range; j <= range; ++j) {
				if (((i + j) % 2 + 2) % 2 == parity)
					offset.push_back({ i, j });
			}
		}
	}

	void addDescription(std::string about, std::string description) {
		if (maxDescriptionLen <= (int) about.size()) {
			for (auto& it : descriptionLine) {
				int idx = 0;
				while (idx < (int) it.size() && it[idx] != ':') ++idx;
				while (idx < (int) it.size() && it[idx] != ' ') ++idx;
				int idx2 = 0;
				for (int i = 0; i < (int)about.size() - maxDescriptionLen; ++i) it.insert(idx++, " ");
			}
			maxDescriptionLen = about.size();
		}	descriptionLine.push_back(about + ": " + std::string(maxDescriptionLen - about.size(), ' ') + description);
	}
	void addRequirementsDescription() {
		for (auto& it : requirements) {
			auto des = it->descriptionGen();
			addDescription(des.first, des.second);
		}
	}

	void addRequirements(CombatMoveRequirement* requirement) { requirements.push_back(requirement); }
	template <class... other_arg> void addRequirements(CombatMoveRequirement* req, other_arg... other) {
		addRequirements(req); 
		addRequirements(other...);
	}

private:
	virtual inline	std::string descriptionHelper() { std::string ret; for (auto it : descriptionLine) ret += " " + it + "\n"; return ret; }

};

class Attack : public CombatMoveInterface {
public:
	Attack(TARGET_ALIVESTATE targetAliveState, COLOR color, int minTargets, bool bGlobal, std::string name, std::string fantasy, std::string techDescription)
		: CombatMoveInterface(targetAliveState, color, minTargets, bGlobal, name, fantasy, techDescription)
	{}
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
	virtual void triggerEvents(std::vector <std::pair <int, int>> profiles);
};

class Spell : public CombatMoveInterface {
public:
	Spell(TARGET_ALIVESTATE targetAliveState, COLOR color, int minTargets, bool bGlobal, std::string name, std::string fantasy, std::string techDescription)
		: CombatMoveInterface(targetAliveState, color, minTargets, bGlobal, name, fantasy, techDescription)
	{}
	virtual void triggerEvents(std::vector <CombatProfile*> profiles);
	virtual void triggerEvents(std::vector <std::pair <int, int>> profiles);
	void refresh() {
		for (auto& it : requirements) {
			Cooldown* cast = dynamic_cast <Cooldown*> (it);
			if (cast != nullptr && cast->charged < cast->value) cast->charged++;
		}
	}
	std::string getBonusInfo() {
		std::string ans = " ";
		for (auto& it : requirements) {
			Cooldown* cast = dynamic_cast <Cooldown*> (it);
			if (cast != nullptr && cast->charged < cast->value) ans += "(" + intToStr(cast->charged) + ")";
		}	return ans;
	}
};

//	AI
struct AIMove {
	class PrimitiveAIBrain* brain;
	AIMove(PrimitiveAIBrain* brain, float score) : brain(brain), score(score) {}
	float score;
	virtual void call() = 0;
};
struct AIAttack : public AIMove {
	AIAttack(CombatProfile* target, PrimitiveAIBrain* brain, float score, std::pair <int, int> tile);
	CombatProfile* target;
	std::pair <int, int> tile;
	virtual void call() override;
};
struct AISpell : public AIMove {
	AISpell(class Spell* spell, PrimitiveAIBrain* brain, float score, std::pair <int, int> tile);
	Spell* spell;
	std::pair <int, int> tile;
	virtual void call() override;
};

class PrimitiveAIBrain {
public:
	PrimitiveAIBrain(Character* ch);
	virtual ~PrimitiveAIBrain();

	void actionZero();
	void actionOne();
	void actionTwo();
	void actionMinus();

	inline void turnBeginEffects() {
		didNothingThisTurn = true;
	}

	int  front() { return actionQueue.front(); }
	void pop() { actionQueue.pop(); }
	bool empty() { return actionQueue.empty(); }

	void runCombatAI();

	void targetSetup(int count, bool bDistinct, Spell* spell);

	void solveSelector(class InputSingleSelector* selector);

	Spell* getCandidateSpell() { return spell; }

	bool bFreeze;
	bool defaultLastResortResponse;
	bool bSittingDuck;

protected:
	std::queue <int> actionQueue;

	bool didNothingThisTurn;

	std::vector <AIMove*> moves;
	std::pair <int, int> tileAI;

	std::vector <CombatProfile*> targetArr;
	CombatProfile* target;
	Character* ch;
	Spell* spell;

	virtual bool makeLastResortDecision();
	virtual void doLastResortMovement();

	void resetData();

	std::pair <int, int> chooseAttackTile(std::map <std::pair <int, int>, std::pair <int, int>> map, std::set <std::pair <int, int>> forbidden);

	virtual std::pair <int, int> chooseAdvancementTile();
	virtual std::pair <int, int> chooseRetreatTile();

	void attackSetup(CombatProfile* target, std::pair <int, int> tile);
	void attackCheck();
	bool spellCheck(Spell* spell);
	void spellSetup(Spell* spell, std::pair <int, int> tile);

	virtual float parseEventArray(std::vector <class Event*> arr);
	virtual float computeTargetCandidateScore(CombatProfile* target, int mpCost, std::pair <int, int> tile);
	virtual float computeScore(class Spell* spell, std::pair <int, int> tile, int cost);

	virtual bool areEqual(std::pair <std::pair <int, int>, int> candidate, std::pair <std::pair <int, int>, int> old, Spell* spell);
	virtual bool isBetter(std::pair <std::pair <int, int>, int> candidate, std::pair <std::pair <int, int>, int> old, Spell* spell);
	virtual bool isEnemy(CombatProfile* profile);

private:
	int randIdx;
	std::vector <int> randArr;
	void recomputeRandArr() {
		randIdx = 0;
		randArr.clear();
		for (int i = 0; i < 100; ++i)
			randArr.push_back(Rand());
	}
	inline void resetRand() { randIdx = 0; }
	inline int getRand() {
		if (randIdx >= (int)randArr.size()) return Rand();
		return randArr[randIdx++];
	}

	friend struct AIAttack;
	friend struct AISpell;
};
