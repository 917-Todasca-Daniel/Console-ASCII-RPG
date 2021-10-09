#pragma once

#include <vector>
#include <set>
#include <string>

class Character;

class Player {
public:
	Player();

	Character*& getCharacter() { return party[0]; }
	std::vector <Character*> getParty() { return party; }

	std::set <std::string> tags;
	bool checkTag(std::string str) {
		if (tags.find(str) == tags.end())
			return false;
		return true;
	}

	void addToInventory(std::string item) { inventory.push_back(item); }
	const std::vector <std::string>& getInventory() { return inventory; }
	std::string getInventoryStringList() {
		std::string ans = "INVENTORY:\n";
		for (auto& item : inventory) ans += std::string(" - ") + item + '\n';
		return ans;
	}

	std::string getPartyStringList();

private:
	std::vector <Character*> party;
	std::vector <std::string> inventory;

	friend class Debugger;
};