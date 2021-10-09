#pragma once

#include "../Characters.h"


//	PROFILES
struct PaladinProfile : public CombatProfile {
public:
	PaladinProfile(std::string name);
	virtual ~PaladinProfile() {};
};

struct BanditProfile : public CombatProfile {
public:
	BanditProfile(std::string name);
	virtual ~BanditProfile() {};
};

struct MuteProfile : public CombatProfile {
public:
	MuteProfile(std::string name);
	virtual ~MuteProfile() {};
};

struct GolemCatProfile : public CombatProfile {
public:
	GolemCatProfile(std::string name);
	virtual ~GolemCatProfile() {};
};

struct ArcherProfile : public CombatProfile {
public:
	ArcherProfile(std::string name);
	virtual ~ArcherProfile() {};
};

//	CHARACTERS
class CHPaladin : public Character {
public:
	CHPaladin();
	virtual void requestAnimation(std::string name, std::string nextAnim = "") override;
};

class TheMute : public Character {
public:
	TheMute();
	virtual void requestAnimation(std::string name, std::string nextAnim = "") override;
};

class Bandit : public Character {
public:
	Bandit(std::string name);
	virtual void requestAnimation(std::string name, std::string nextAnim = "") override;
};

class Archer : public Character {
public:
	Archer(std::string name);
	virtual void requestAnimation(std::string request, std::string next = "") override;
};

class GolemCat : public Character {
public:
	GolemCat(std::string name);
	virtual void requestAnimation(std::string request, std::string next = "") override;
};
