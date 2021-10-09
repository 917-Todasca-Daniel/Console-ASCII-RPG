#include "AvatarCollection.h"

#include "engine/ScreenObject.h"

std::map <std::string, ScreenObject*> AvatarCollection::map = std::map <std::string, ScreenObject*>();

ScreenObject* AvatarCollection::getAvatar(std::string name) {
	auto it = map.find(name);
	if (it == map.end()) {
		ScreenObject* obj = new ScreenObject(0, 0, 10000, "avatars/" + name);
		obj->setName("avatar");
		obj->hide();
		map[name] = obj;
		return obj;
	}	return it->second;
}

void AvatarCollection::clearMemory() {
	for (auto& it : map) {
		it.second->destroy();
	}	map.clear();
}
