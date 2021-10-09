#ifndef AVATARCOLLECTION_H
#define AVATARCOLLECTION_H

#include <map>
#include <string>

class ScreenObject;

class AvatarCollection {
public:
	AvatarCollection() {}

	static ScreenObject* getAvatar(std::string name);

	static void clearMemory();

private:
	static std::map <std::string, ScreenObject*> map;

	friend class GodClass;
	friend class Debugger;
};

#endif	// AVATARCOLLECTION_H

