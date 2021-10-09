#include "AnimatorTable.h"

#include "Animator.h"

#include <fstream>
#include <sstream>

#define BUFFER_SIZE 105

AnimatorTable::AnimatorTable(std::string tasc_name) {
	tasc_name = "resources/" + tasc_name + ".tasc";
	std::ifstream src(tasc_name);
	if (src) {
		int px, py; src >> px >> py;
		std::string animName;
		char str[BUFFER_SIZE];
		while (src >> animName) {
			nameMap[animName] = storedAnimations.size();
			storedAnimations.push_back(std::vector <Frame>());
			src.get(), src.getline(str, BUFFER_SIZE);
			int num;
			std::stringstream ss(str);
			while (!ss.eof() && ss >> num)
				storedAnimations.back().push_back({ num, std::vector <OutputData>() });

			for (int i = 0; i < (int)storedAnimations.back().size(); ++i) {
				(storedAnimations.back())[i].data = VisualObject::readOutputDataFromFile(src);
				for (auto& it : (storedAnimations.back())[i].data)
					it.x -= px, it.y -= py;
			}
		}
	}   src.close();
}

AnimatorTable::~AnimatorTable() {}

void AnimatorTable::animate(Animator* anim, int tableIndex) {
	if (!anim) return;
	anim->frames = storedAnimations[tableIndex];
}

void AnimatorTable::animate(Animator* anim, std::string tableName) {
	if (nameMap.find(tableName) == nameMap.end()) return;
	animate(anim, nameMap[tableName]);
}

