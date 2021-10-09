#include "Player.h"

#include "CharactersCore.h"
#include "AvatarCollection.h"
#include "engine/ScreenObject.h"
#include "engine/consolewizard.h"

Player::Player() {
	party.push_back(new TheMute());
	party.push_back(new Archer("Archer"));
}

std::string Player::getPartyStringList() {
	std::string ans = "PARTY:\n";
	AvatarCollection::getAvatar("avatar")->show();
	AvatarCollection::getAvatar("avatar")->setPosition(getwidth()-30, 4);
	AvatarCollection::getAvatar("avatar")->color = GRAY;
	return ans;
}
