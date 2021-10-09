#include "Characters.h"

#include "engine/Animator.h"
#include "engine/AnimatorTable.h"
#include "Behaviours.h"
#include "engine/Menu.h"
#include "engine/Debugger.h"
#include "engine/Timer.h"

#include "Encounter.h"

#include <algorithm>

int StonePillarProfile::counter = 0;
std::string CombatMoveInterface::vulnerable_description = "marked by x; consumed after use";
int Character::chs_count = 0;
CharacterSpeakLinesTracker* Character::lineTrackers[Character::SPEAK_CNT] = { };



//	COMBAT PROFILE
CombatProfile::CombatProfile(std::string name, SENTIENCE sentience, int AP, int startAP, int regenAP, int MP, int startMP, int regenMP, int HP, int DR) 
	: name(name), sentience(sentience), AP(AP), startAP(startAP), regenAP(regenAP), MP(MP), startMP(startMP), regenMP(regenMP), HP(HP), DR(DR) { 
	prev = { false, 0 };
	pres = { false, 0 };
}
CombatProfile::~CombatProfile() {
	if (init)	 delete init;
	if (brainAI) delete brainAI;
	for (auto it : attackArr) delete it;
	for (auto it : spellArr)  delete it;
}

void CombatProfile::resetCooldowns() {
	for (auto& it : spellArr) { it->resetCooldown(); }
}
void CombatProfile::applyMovementCost(int value) {
	MP -= value;
}
void CombatProfile::turnBeginEffects() {
	if (brainAI) brainAI->turnBeginEffects(); 

	if (!bFirstTurn) {
		AP = ((AP += regenAP) >= init->AP ? init->AP : AP);
		MP = ((MP += regenMP) >= init->MP ? init->MP : MP);
		AP -= fatigue;
		MP -= fatigueMP;
		fatigue = 0;
		fatigueMP = 0;
	}	bFirstTurn = false;

	for (auto& it : spellArr) it->refresh();
}


//	CHARACTER
Character::Character(std::string name) {
	if (++chs_count == 1) {
		for (int i = 0; i < SPEAK_CNT; ++i) {
			lineTrackers[i] = new CharacterSpeakLinesTracker(120);
		}
	}
	setName("Character");
	x = y	= 0;
	bHide	= false;
	cp		= nullptr;
}
Character::~Character() {
	for (auto it : parts) {
		if (it.second) delete it.second;
	}
}
void Character::destroy() {
	if (--chs_count == 1) {
		for (int i = 0; i < SPEAK_CNT; ++i) {
			lineTrackers[i]->destroy();
			lineTrackers[i] = nullptr;
		}
	}
	PrimitiveObject::destroy();
	for (auto it : parts) {
		if (it.first) it.first->destroy();
	}
	if (timer) timer->destroy();
}

void Character::loop(float delta) {
	PrimitiveObject::loop(delta);

	random_shuffle(begin(dmgVisualOffsets), end(dmgVisualOffsets));

	visualIdx = 0;

	if (bHide) hideVisual();
	else showVisual();

	if (cp) {
		int val = (cp->prev.bMoving ^ cp->pres.bMoving) * (cp->pres.bMoving + 1);
		if (val == 1)		requestAnimation("idle");
		else if (val == 2) {
			std::string str = "walk" + intToStr(cp->pres.movementDirection);
			requestAnimation(str, str);
		}

		cp->prev = cp->pres;
	}
}

void Character::requestAnimation(std::string name, std::string nextAnim) {
	if (ENCOUNTER && ENCOUNTER->bSimMode == true) return;

	for (auto it : parts) {
		it.first->reset(), it.second->animate(it.first, name), it.first->play();
		it.first->setAnimTable(it.second);
	}
	if (name == "attack") {
		if (cp->checkAttackFlip()) {
			for (auto it : parts)
				it.first->flipX(flipAxis);
		}
	}
	if (name == "idle") nextAnim = "idle";
	if (nextAnim != "") {
		for (auto it : parts) it.first->setNextAnim(nextAnim);
	}
	else {
		for (auto it : parts) it.first->setAnimTable(nullptr);
	}
	for (auto it : parts) it.first->setStartFrame(0);
}

void Character::createAttackVisualEffects(int dmg) {
	if (dmg != 0) {
		new FlashBehaviour <Character>(5, 0.1f, 0.1f, this);
		new TextFloatingPopup("-" + intToStr(dmg), 1.0f + rand() % 10 * 0.1f, 8.0f + rand() % 5 * 0.1f, x + dmgVisualOffsets[visualIdx].first, y + dmgVisualOffsets[visualIdx].second);
		visualIdx = (visualIdx + 1) % dmgVisualOffsets.size();
	}
}

void Character::buff(std::string desc, COLOR color) {
	new FlashBehaviour <Character>(5, 0.1f, 0.1f, this);
	new TextFloatingPopup(desc, 1.0f + rand() % 10 * 0.1f, 8.0f + rand() % 5 * 0.1f, x + dmgVisualOffsets[visualIdx].first, y + dmgVisualOffsets[visualIdx].second, color);
	visualIdx = (visualIdx + 1) % dmgVisualOffsets.size();
}

void Character::speak(const std::string& msg, COLOR color) {
	new TextFloatingPopup(msg, 2.0f + rand() % 10 * 0.1f, 6.0f + rand() % 5 * 0.1f, x + head_x, y + head_y, 100, color);
}

void Character::speak(SPEAKTYPES type, COLOR color) {
	if (cp->sentience == OBJECT) return;
	if (type == TAKEDMG) {
		if (cp->HP <= 0) return;
		if (cp->HP < cp->init->HP / 5) type = CRITICAL;	
		auto chs = ENCOUNTER->getCharacters();
		random_shuffle(begin(chs), end(chs));
		for (auto ch : chs) {
			if (ch != this && ch->cp->sentience == cp->sentience) {
				ch->speak(SUPPORT);
				break;
			}
		}
	}
	if (!lineTrackers[type]->roll()) return;
	std::string msg = speakMessages[type][randBetween(0, speakMessages[type].size())];
	new TextFloatingPopup(msg, 2.0f + rand() % 10 * 0.1f, 6.0f + rand() % 5 * 0.1f, x + head_x, y + head_y, 100, color);
}

void Character::overrideRenderPlane() {
	for (auto& it : parts) it.first->overrideRenderPlane(20);
}
void Character::resetPriority() {
	for (auto& it : parts) it.first->overrideRenderPlane(10);
}
void Character::setColor(Color color) {
	for (auto part : parts)
		part.first->color = color;
}
void Character::flipX() {
	for (auto& it : parts) {
		it.first->flipX(flipAxis);
	}
}

void Character::show() { bHide = false; showVisual(); }
void Character::hide() { bHide = true;  hideVisual(); }
void Character::showVisual() {
	for (auto it : parts) it.first->show();
}
void Character::hideVisual() {
	for (auto it : parts) it.first->hide();
}
void Character::hideVisual(float duration) {
	hide();
	timer = new Timer(duration, this, &Character::show);
}
void Character::setPosition(int x, int y) {
	this->x = x, this->y = y;
	for (auto it : parts) it.first->setPosition(x, y);
}

void Character::openTable(std::string name) {
	parts.push_back({ new Animator("dummy", 10, false), new AnimatorTable(name) });
	parts.back().first->setPosition(-1000, -1000);
	parts.back().first->setName("part");
}

Object::Object(std::string name) : Character(name) { }

StonePillar::StonePillar(std::string name) : Object(name) {
	setCombatProfile(new StonePillarProfile());
	openTable("pillar");
	requestAnimation("idle");
	show();
}

void Spell::triggerEvents(std::vector<CombatProfile*> profiles) {
	if (ENCOUNTER->bSimMode) return;
	source->lastMove = this;
	for (auto it : requirements) it->applyConsequences(source);
	Event* spellcastEvent = new Event(); spellcastEvent->src = source;
	ENCOUNTER->updateHistory <SpellcastLog>(spellcastEvent);
};
void Spell::triggerEvents(std::vector<std::pair <int, int>> profiles) {
	if (ENCOUNTER->bSimMode) return;
	source->lastMove = this;
	for (auto it : requirements) it->applyConsequences(source);
	Event* spellcastEvent = new Event(); spellcastEvent->src = source;
	ENCOUNTER->updateHistory <SpellcastLog>(spellcastEvent);
};
void Attack::triggerEvents(std::vector<CombatProfile*> profiles) {
	if (ENCOUNTER->bSimMode) return;
	source->lastMove = this;
	for (auto it : requirements) it->applyConsequences(source);
	Event* attackcastEvent = new Event(); attackcastEvent->src = source;
	ENCOUNTER->updateHistory <AttackcastLog>(attackcastEvent);
}
void Attack::triggerEvents(std::vector <std::pair <int, int>> profiles) {
	if (ENCOUNTER->bSimMode) return;
	source->lastMove = this;
	for (auto it : requirements) it->applyConsequences(source);
	Event* attackcastEvent = new Event(); attackcastEvent->src = source;
	ENCOUNTER->updateHistory <AttackcastLog>(attackcastEvent);
}

std::vector<CombatProfile*> CombatMoveInterface::targetsInRange(Character* caster, std::vector<Character*> characters) {
	std::vector <CombatProfile*> ans;

	CombatProfile* cp = caster->cp;
	std::set <std::pair <int, int>> targetPositions;
	if (!bGlobal) for (auto it : rangeTiles(cp->getTile().first, cp->getTile().second)) targetPositions.insert(it);
	for (auto it : characters)
		if (checkTargetability(cp, it->cp) && (bGlobal || targetPositions.find(it->cp->getTile()) != targetPositions.end()))
			ans.push_back(it->cp);

	return ans;
}
ValidityCode CombatMoveInterface::checkMoveValidity(Character* caster, std::vector<Character*> characters) {
	validTargets.clear();

	for (auto it : targetsInRange(caster, characters))
		validTargets.push_back(it->ch);

	std::vector <Character*> cleaner;
	if (targetAliveState != ALL_STATES) for (auto it : validTargets) {
		if (targetAliveState == 1 + it->cp->bDead)
			cleaner.push_back(it);
	}	validTargets = cleaner;

	if ((int)validTargets.size() < minTargets) return VC_NO_TARGETS;
	return VC_OK;
}

//	AI
PrimitiveAIBrain::PrimitiveAIBrain(Character* ch) : ch(ch) {
	bFreeze = false;
	target = nullptr;
	spell = nullptr;
	randIdx = 0;
	didNothingThisTurn = false;
	defaultLastResortResponse = true;
}
PrimitiveAIBrain::~PrimitiveAIBrain() {}

void PrimitiveAIBrain::resetData() {
	tileAI = { -1, -1 };
	spell = nullptr;
	target = nullptr;
}

void PrimitiveAIBrain::runCombatAI() {
	if (bSittingDuck) actionQueue.push(-1);
	if (bFreeze) return;
	if (ch->cp->bDead) {
		while (!actionQueue.empty())
			actionQueue.pop();
		actionQueue.push(-1);
		return;
	}
	
	recomputeRandArr();
	resetData();

	attackCheck();
	for (auto& sp : ch->cp->getSpells())
		spellCheck(sp);

	std::vector <std::pair <float, int>> arr;
	for (int i = 0; i < (int)moves.size(); ++i) {
		if (moves[i]->score > 0)
			arr.push_back({ moves[i]->score, i });
		//	DEBUG_PRINT(intToStr(moves[i]->score), 5.0f);
	}

	if (arr.empty()) {
		if (didNothingThisTurn)
			doLastResortMovement();
		else actionQueue.push(-1);
	}
	else {
		didNothingThisTurn = false;
		moves[randomArrElementWeighted(arr)]->call();
	}

	for (auto it : moves) delete it;
	moves.clear();
}

void PrimitiveAIBrain::attackCheck() {
	if (ch->cp->getAttacks().empty()) return;
	Attack* atk = ch->cp->getAttacks()[0];
	CombatProfile* cp = ch->cp;

	if (cp->MP == 0 || !atk->checkRequirements(cp)) return;

	std::vector <Character*> chList = ENCOUNTER->getCharacters();
	std::vector <std::tuple <CombatProfile*, int, std::pair <int, int>>> arr;

	for (auto targetCandidate : ENCOUNTER->getProfiles()) {
		if (!isEnemy(targetCandidate)) continue;
		auto map = LAYOUTS->AIHelper(GRID, targetCandidate->x(), targetCandidate->y(), cp->x(), cp->y());
		auto forbidden = LAYOUTS->getForbiddenDestinationTiles(GRID);

		auto tile = chooseAttackTile(map, forbidden);
		
		if (abs(targetCandidate->x() - tile.first) + abs(targetCandidate->y() - tile.second) <= cp->attackRange)
			arr.push_back(std::make_tuple(targetCandidate, map[tile].second, tile));
	};

	if (arr.empty()) return;

	CombatProfile* targetCandidate = nullptr;
	std::pair <int, int> tile;
	int cost;

	std::vector <std::pair <float, int>> weightedArr;
	for (int i = 0; i < (int) arr.size(); ++i) {
		std::tie(targetCandidate, cost, tile) = arr[i];
		float score = computeTargetCandidateScore(targetCandidate, cost, tile);
		weightedArr.push_back({ score, i });
	}

	std::tie(targetCandidate, cost, tile) = arr[randomArrElementWeighted(weightedArr)];

	float score = computeTargetCandidateScore(targetCandidate, cost, tile);
	moves.push_back(new AIAttack(targetCandidate, this, score, tile));
}

bool PrimitiveAIBrain::spellCheck(Spell* candidate) {
	CombatProfile* cp = ch->cp;

	auto map = LAYOUTS->calcDist(GRID, cp->x(), cp->y());
	auto forbidden = LAYOUTS->getForbiddenDestinationTiles(GRID);
	forbidden.erase(cp->getTile());

	std::vector <std::pair <std::pair <int, int>, int>> tiles;
	for (auto& it : map) {
		if (!ENCOUNTER->checkValability(it.first) && it.first != cp->getTile()) continue;
		if (forbidden.find(it.first) == forbidden.end() && it.second <= cp->MP) {
			int x = it.first.first;
			int y = it.first.second;
			int cpy_x = cp->x();
			int cpy_y = cp->y();
			int mpCost = it.second;
			int v = it.second;

			cp->setTile(POINT(x, y));
			cp->MP -= mpCost;
			if (candidate->checkMoveValidity(ch, ENCOUNTER->getCharacters()) == VC_OK
				&& candidate->checkRequirements(cp)) {
				if (tiles.empty()) {
					tiles.push_back(it);
				}
				else {
					if (areEqual(it, tiles.back(), candidate))
						tiles.push_back(it);
					else if (isBetter(it, tiles.back(), candidate)) {
						tiles.clear();
						tiles.push_back(it);
					}
				}
			}

			cp->setTile(POINT(cpy_x, cpy_y));
			cp->MP += mpCost;
		}
	}

	if (tiles.empty()) return false;

	tileAI = tiles[Rand() % tiles.size()].first;
	moves.push_back(new AISpell(candidate, this, computeScore(candidate, tileAI, map[tileAI]), tileAI));

	return true;
}

std::pair<int, int> PrimitiveAIBrain::chooseAttackTile(std::map<std::pair<int, int>, std::pair<int, int>> map, std::set<std::pair<int, int>> forbidden) {
	std::pair <std::pair <int, int>, std::pair <int, int>> ans = { {-1, -1}, {(int)2e9, (int)2e9} };

	CombatProfile* cp = ch->cp;

	forbidden.erase(cp->getTile());
	std::vector <std::pair <int, int>> poss;
	poss.push_back(ans.first);
	for (auto& it : map) {
		if (!ENCOUNTER->checkValability(it.first) && it.first != cp->getTile()) continue;
		if (forbidden.find(it.first) == forbidden.end() && it.second.second <= cp->MP) {
			int x = it.first.first;
			int y = it.first.second;
			int cpy_x = cp->x();
			int cpy_y = cp->y();
			int mpCost = it.second.second;

			auto cpy = cp->getTile();

			cp->setTile(POINT(x, y));
			cp->MP -= mpCost;

			if (cp->getAttacks()[0]->checkMoveValidity(ch, ENCOUNTER->getCharacters()) == VC_OK && cp->getAttacks()[0]->checkRequirements(ch->cp)) {
				if (ans.second > it.second)
					poss.clear(), poss.push_back(it.first), ans = it;
				else if (ans.second == it.second) {
					poss.push_back(it.first);
				}
			}

			cp->setTile(POINT(cpy_x, cpy_y));
			cp->MP += mpCost;
		}
	}

	if (poss.empty()) {
		return std::pair <int, int>(-1, -1);
	}
	return randomArrElement(poss);
}


float PrimitiveAIBrain::computeTargetCandidateScore(CombatProfile* target, int mpCost, std::pair<int, int> tile) {
	float score = (float) (100 - 1.0 * target->HP / target->init->HP * 100.0);
	return score;
}
float PrimitiveAIBrain::computeScore(Spell* spell, std::pair<int, int> tile, int cost) {
	ENCOUNTER->turnOnSim();
	CombatProfile* cp = ch->cp;
	int x = tile.first;
	int y = tile.second;
	int cpy_x = cp->x();
	int cpy_y = cp->y();
	cp->setTile(POINT(x, y));
	cp->MP -= cost;

	spell->castMove(cp);
	spell->triggerEvents(targetArr);

	auto arr = ENCOUNTER->simArr;
	float score = parseEventArray(arr);

	ENCOUNTER->turnOffSim();
	cp->setTile(POINT(cpy_x, cpy_y));
	cp->MP += cost;

	return score;
}

float PrimitiveAIBrain::parseEventArray(std::vector <Event*> arr) {
	float score = 0;
	for (auto it : arr) {
		FixedDamageEvent* fde = dynamic_cast <FixedDamageEvent*> (it);
		if (fde != nullptr) {
			score += fde->getValue();
			continue;
		}
		TrueScallingMAXHPDamageEvent* scallingde = dynamic_cast <TrueScallingMAXHPDamageEvent*> (it);
		if (scallingde != nullptr) {
			score += scallingde->getValue();
			continue;
		}
	}
	return score;
}

void PrimitiveAIBrain::attackSetup(CombatProfile* targetCandidate, std::pair <int, int> tile) {
	tileAI = tile;
	target = targetCandidate;
	actionQueue.push(0);
	actionQueue.push(1);
}
void PrimitiveAIBrain::spellSetup(Spell* spellCandidate, std::pair <int, int> tile) {
	tileAI = tile;
	actionQueue.push(0);
	actionQueue.push(2);
	spell = spellCandidate;
}

bool PrimitiveAIBrain::isEnemy(CombatProfile* profile) {
	return profile->sentience == PLAYER;
}
bool PrimitiveAIBrain::areEqual(std::pair<std::pair<int, int>, int> candidate, std::pair<std::pair<int, int>, int> old, Spell *spell) {
	return candidate.second == old.second;
}
bool PrimitiveAIBrain::isBetter(std::pair<std::pair<int, int>, int> candidate, std::pair<std::pair<int, int>, int> old, Spell* spell) {
	return candidate.second < old.second;
}

void PrimitiveAIBrain::solveSelector(InputSingleSelector* selector) {
	int v = getRand() % selector->choices.size();
	selector->choice = selector->choices[v];
}

void PrimitiveAIBrain::targetSetup(int count, bool bDistinct, Spell *spell) {
	resetRand();
	targetArr.clear();

	spell->checkMoveValidity(ch, ENCOUNTER->getCharacters());
	auto arrCopy = spell->targetsInRange(this->ch, ENCOUNTER->getCharacters());

	for (int i = 0; i < count; ++i) {
		if (arrCopy.empty()) return;
		int v = getRand() % arrCopy.size();
		targetArr.push_back(arrCopy[v]);

		if (bDistinct) arrCopy.erase(arrCopy.begin() + v);
	}
}

bool PrimitiveAIBrain::makeLastResortDecision() {
	return defaultLastResortResponse;
}
void PrimitiveAIBrain::doLastResortMovement() {
	didNothingThisTurn = false;
	
	std::pair <int, int> tile(-1, -1);
	if (makeLastResortDecision())
		tile = chooseAdvancementTile();
	else
		tile = chooseRetreatTile();

	if (tile.first != -1) {
		actionQueue.push(0);
		tileAI = tile;
	}
	else {
		actionQueue.push(-1);
	}
}

std::pair<int, int> PrimitiveAIBrain::chooseAdvancementTile() {
	std::pair<int, int> ans(-1, -1);
	CombatProfile* cp = ch->cp;
	CombatProfile* target = nullptr;

	auto aux = LAYOUTS->calcDist(GRID, cp->x(), cp->y());

	int dist = (int) 2e9;
	for (auto& it : ENCOUNTER->getProfiles()) {
		if (isEnemy(it) && dist > aux[it->getTile()]) {
			target = it;
			dist = aux[it->getTile()];
		}
	}

	auto map = LAYOUTS->AIHelper(GRID, target->x(), target->y(), cp->x(), cp->y());
	auto forbidden = LAYOUTS->getForbiddenDestinationTiles(GRID);
	forbidden.erase(cp->getTile());

	int maxMP = cp->MP;
	int min = (int) 2e9;

	for (auto it : map) {
		if (!ENCOUNTER->checkValability(it.first) && it.first != cp->getTile()) continue;
		if (forbidden.find(it.first) != forbidden.end()) continue;
		if (it.second.second <= maxMP) {
			if (it.second.first < min) {
				min = it.second.first;
				ans = it.first;
			}
			else if (it.second.first == min) {
				int v1 = abs(cp->x() - ans.first) * abs(cp->y() - ans.second);
				int v2 = abs(cp->x() - it.first.first) * abs(cp->y() - it.first.second);
				if (v2 > v1) ans = it.first;
			}
		}
	}

	if (ans == cp->getTile()) ans = { -1, -1 };
	return ans;	
}
std::pair<int, int> PrimitiveAIBrain::chooseRetreatTile() {
	std::pair<int, int> ans(-1, -1);
	CombatProfile* cp = ch->cp;
	CombatProfile* target = nullptr;

	auto aux = LAYOUTS->calcDist(GRID, cp->x(), cp->y());

	int dist = (int) 2e9;
	for (auto& it : ENCOUNTER->getProfiles()) {
		if (isEnemy(it) && dist > aux[it->getTile()]) {
			target = it;
			dist = aux[it->getTile()];
		}
	}

	if (target == nullptr) return ans;
	auto map = LAYOUTS->AIHelper(GRID, target->x(), target->y(), cp->x(), cp->y());
	auto forbidden = LAYOUTS->getForbiddenDestinationTiles(GRID);
	forbidden.erase(cp->getTile());

	int maxMP = cp->MP;
	int max = (int) -2e9;

	for (auto it : map) {
		if (!ENCOUNTER->checkValability(it.first) && it.first != cp->getTile()) continue;
		if (forbidden.find(it.first) != forbidden.end()) continue;
		if (it.second.second <= maxMP) {
			if (it.second.first > max) {
				max = it.second.first;
				ans = it.first;
			}
			else if (it.second.first == max) {
				int v1 = abs(target->x() - ans.first) * abs(target->y() - ans.second);
				int v2 = abs(target->x() - it.first.first) * abs(target->y() - it.first.second);
				if (v2 < v1) ans = it.first;
			}
		}
	}

	if (ans == cp->getTile()) ans = { -1, -1 };
	return ans;
}

void PrimitiveAIBrain::actionZero() {
	CombatProfile* cp = ch->cp;

	if (tileAI != std::pair <int, int> (-1, -1)) {
		if (tileAI == cp->getTile()) return;
		ENCOUNTER->pressMovement();
		ENCOUNTER->pressTile(tileAI);
		return;
	}
	if (target == nullptr) return;
}
void PrimitiveAIBrain::actionOne() {
	CombatProfile* cp = ch->cp;

	Attack* atk = cp->getAttacks()[0];
	if (atk->checkRequirements(cp)) {
		int dist = abs(cp->x() - target->x()) + abs(cp->y() - target->y());
		if (dist <= cp->attackRange) {
			ENCOUNTER->pressAttack(target);

			if (atk->checkRequirements(cp)) {
				actionQueue.push(1);
			}
		}
	}
}
void PrimitiveAIBrain::actionTwo() {
	resetRand();
	ENCOUNTER->pressSkills();
}
void PrimitiveAIBrain::actionMinus() {
	ENCOUNTER->pressEndTurn();
}

AIAttack::AIAttack(CombatProfile* target, PrimitiveAIBrain* brain, float score, std::pair <int, int> tile) : target(target), tile(tile), AIMove(brain, score) {}
AISpell::AISpell(Spell *spell, PrimitiveAIBrain* brain, float score, std::pair <int, int> tile) : spell(spell), tile(tile), AIMove(brain, score) {}

void AIAttack::call() {
	brain->attackSetup(target, tile);
}
void AISpell::call() {
	brain->spellSetup(spell, tile);
}
