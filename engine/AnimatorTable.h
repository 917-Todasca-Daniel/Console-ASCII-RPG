#ifndef ANIMATORTABLE_H
#define ANIMATORTABLE_H

#include "GarbageTruck.h"

#include <string>
#include <vector>
#include <map>

class Animator;
struct Frame;

class AnimatorTable : public GarbageTruck
{
public:
	AnimatorTable(std::string tasc_name);
	virtual ~AnimatorTable();

	void animate(Animator* anim, int tableIndex);
	void animate(Animator* anim, std::string tableName);

protected:
	std::vector <std::vector <Frame>> storedAnimations;
	std::map <std::string, int> nameMap;

private:

	friend class GodClass;
	friend class Debugger;
};

#endif // ANIMATORTABLE_H
