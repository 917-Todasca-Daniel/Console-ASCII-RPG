#include "Scripts.h"

#include "engine/GodClass.h"
#include "engine/ScreenObject.h"
#include "engine/ScreenObjectTable.h"
#include "engine/Controller.h"
#include "engine/Windows.h"

#include "Narration.h"
#include "AvatarCollection.h"
#include "Player.h"
#include "Encounter.h"
#include "Behaviours.h"

Player* ScriptUtilities::player = nullptr;
Narration* ScriptUtilities::narrator = nullptr;
std::ofstream ScriptUtilities::logOutput = std::ofstream("scripts/markup_log.txt");

Script* ScriptUtilities::script = nullptr;

template <typename container>
MapLayout::MapLayout(const container &arr, int map_x, int map_y) {
	scbt = new ScreenObjectTable("maptiles");
	
	for (auto it : arr) {
		map[it] = "square";
		objMap[it] = new ScreenObject(map_x + it.first * 4, map_y + it.second * 2, 500, "");
	}
}
MapLayout::~MapLayout() {
	for (auto it : objMap) it.second->destroy();
}

void MapLayout::setRoomShape(std::pair<int, int> pair, std::string shape) {
	map[pair] = shape;
}
void MapLayout::travelTo(std::pair<int, int> pair) {
	scbt->updateShape(objMap[pair], map[pair]);

	int x1 = pair.first, y1 = pair.second;
	for (auto it : objMap) {
		int x2 = it.first.first, y2 = it.first.second;
		if (it.second->data.size() > 0) continue;

		if (y1 == y2) {
			if (x2 == x1 + 1) {
				scbt->updateShape(it.second, "unknown_right");
			}
			else if (x2 == x1 - 1) {
				scbt->updateShape(it.second, "unknown_left");
			}
		}
		else
			if (x1 == x2) {
				if (y2 == y1 + 1) {
					scbt->updateShape(it.second, "unknown_down");
				}
				else if (y2 == y1 - 1) {
					scbt->updateShape(it.second, "unknown_up");
				}
			}
	}
}

void MapLayout::hide() {
	for (auto it : objMap)
		it.second->hide();
}

void MapLayout::show() {
	for (auto it : objMap)
		it.second->show();
}

ScriptGraph* ScriptUtilities::readFile(std::string filename) {
	ScriptUtilities::log("Started reading script file: " + filename + ".\n");
	ScriptGraph *graph = new ScriptGraph();
	std::ifstream file("scripts/" + filename + ".txt"); 
	std::string line, options, flag, ptext;
	int x = 0, y = 0;
	int line_index = 0;
	while (true) {
		std::getline(file, line);
		line_index++;
		if (line[0] == '%') break;
		if (line[0] == '/') continue;

		std::vector <std::string> arr, tags;
		flag = ptext = "";

		if (line[0] == '*') {
			ptext = line.substr(1);
			std::getline(file, line);
			line_index++;
		}
		if (line[0] == '!') {
			flag = line.substr(1);
			line_index++;
			std::getline(file, line);
		}
		if (line[0] == '~') {
			if (ptext != "") {
				ScriptUtilities::log("Found a paragraph containing both paragraph and menu options, line: " + intToStr(line_index) + ".\n");
			}
			arr = ScriptUtilities::parseOptions(line.substr(1));
			std::getline(file, line);
			line_index++;
		}
		while (line[0] == '?') {
			tags.push_back(line.substr(1));
			std::getline(file, line);
			line_index++;
		}
		if (line[0] == '[') {
			auto arr2 = split(line.substr(1), ' ');
			x = strToInt(arr2[0]);
			y = strToInt(arr2[1]);
			graph->tiles.insert({ x, y });
			std::getline(file, line);
			line_index++;
		}

		if (line[0] == '#') line = line.substr(1);
		else {
			if (line.size() == 0) ScriptUtilities::log("Empty line ~" + intToStr(line_index) + ", was that the intention?\n");
			else if (!isalpha(line[0]) and !isdigit(line[0]) and line[0] != '.') ScriptUtilities::log("Possible formatting error for paragraph at line ~" + intToStr(line_index) + ", found non-alpha or # character: " + line + ".\n");
			line = "    " + line;
		}
		graph->addNode({ std::pair <int, int> (x, y), -1, line, flag, tags, arr, ptext });
	}	file.close();
	ScriptUtilities::log("Finished reading script file: " + filename + ".\n\n");
	return graph;
}

std::vector<std::string> ScriptUtilities::parseOptions(std::string optionsline) {
	return split(optionsline, '$');
}
void ScriptUtilities::readLinkFile(std::string filename, ScriptGraph*& graph) {
	ScriptUtilities::log("Started reading: " + filename + ".\n");
	std::ifstream file("scripts/links/" + filename + ".txt");
	std::string line;
	int line_index = 0;
	while (true) {
		line_index++;
		std::getline(file, line);
		if (line[0] == '%') break;
		if (line[0] == '/') { line_index++; continue; }
		if (line[0] == '&') {
			auto parent = line.substr(1);
			std::getline(file, line);
			line_index++;
			int num = line[0] - '0';

			auto map1 = ScriptUtilities::parseLinkString(parent);

			try {
				graph->checkParentReferenceValidity(parent, num);
			}
			catch (std::string invalid) {
				ScriptUtilities::log("Error met on line ~" + intToStr(line_index) + ", file " + filename + ": invalid paragraph ID: " + invalid + ".\n");
			}

			for (int i = 0; i < num; ++i) {
				std::getline(file, line);
				line_index++;
				ParagraphConditionChecker* checker = nullptr;

				if (line == "-") continue;

				if (i == 0) checker = new EndcodeValueChecker <1>();
				if (i == 1) checker = new EndcodeValueChecker <2>();
				if (i == 2) checker = new EndcodeValueChecker <3>();
				if (i == 3) checker = new EndcodeValueChecker <4>();
				if (i == 4) checker = new EndcodeValueChecker <5>();
				if (i == 5) checker = new EndcodeValueChecker <6>();
				if (i == 6) checker = new EndcodeValueChecker <7>();
				if (i == 7) checker = new EndcodeValueChecker <8>();
				try {
					graph->addEdge(parent, line, checker, (i == 0));
				}
				catch (std::string invalid) {
					ScriptUtilities::log("Error met on line ~" + intToStr(line_index) + ", file " + filename + ": invalid paragraph ID: " + invalid + ".\n");
				}
			}
		}
		else {
			auto arr = split(line, '-');

			for (int i = 0; i + 1 < (int)arr.size(); ++i) {
				auto map = ScriptUtilities::parseLinkString(arr[i]);

				int code = 0;
				if (map.find("code") != map.end()) {
					code = strToInt(map["code"]);
				}
				ParagraphConditionChecker* checker = nullptr;
				
				if (map.find("tag") != map.end()) {
					auto str = map["tag"];
					if (str[0] == '!') {
						str = str.substr(1);
						if (code == 0) checker = new EndcodeValueChecker <0>(ScriptUtilities::script, &Script::checkMinusTag, str);
						if (code == 1) checker = new EndcodeValueChecker <1>(ScriptUtilities::script, &Script::checkMinusTag, str);
						if (code == 2) checker = new EndcodeValueChecker <2>(ScriptUtilities::script, &Script::checkMinusTag, str);
						if (code == 3) checker = new EndcodeValueChecker <3>(ScriptUtilities::script, &Script::checkMinusTag, str);
						if (code == 4) checker = new EndcodeValueChecker <4>(ScriptUtilities::script, &Script::checkMinusTag, str);
						if (code == 5) checker = new EndcodeValueChecker <5>(ScriptUtilities::script, &Script::checkMinusTag, str);
						if (code == 6) checker = new EndcodeValueChecker <6>(ScriptUtilities::script, &Script::checkMinusTag, str);
						if (code == 7) checker = new EndcodeValueChecker <7>(ScriptUtilities::script, &Script::checkMinusTag, str);
						if (code == 8) checker = new EndcodeValueChecker <8>(ScriptUtilities::script, &Script::checkMinusTag, str);
					}
					else {
						if (code == 0) checker = new EndcodeValueChecker <0>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
						if (code == 1) checker = new EndcodeValueChecker <1>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
						if (code == 2) checker = new EndcodeValueChecker <2>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
						if (code == 3) checker = new EndcodeValueChecker <3>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
						if (code == 4) checker = new EndcodeValueChecker <4>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
						if (code == 5) checker = new EndcodeValueChecker <5>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
						if (code == 6) checker = new EndcodeValueChecker <6>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
						if (code == 7) checker = new EndcodeValueChecker <7>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
						if (code == 8) checker = new EndcodeValueChecker <8>(ScriptUtilities::script, &Script::checkTag, map["tag"]);
					}
				}
				else {
					if (code == 0) checker = new EndcodeValueChecker <0>();
					if (code == 1) checker = new EndcodeValueChecker <1>();
					if (code == 2) checker = new EndcodeValueChecker <2>();
					if (code == 3) checker = new EndcodeValueChecker <3>();
					if (code == 4) checker = new EndcodeValueChecker <4>();
					if (code == 5) checker = new EndcodeValueChecker <5>();
					if (code == 6) checker = new EndcodeValueChecker <6>();
					if (code == 7) checker = new EndcodeValueChecker <7>();
					if (code == 8) checker = new EndcodeValueChecker <8>();
				}

				try {
					graph->addEdge(arr[i], ScriptUtilities::cutMarks(arr[i + 1]), checker);
				}
				catch (std::string invalid) {
					ScriptUtilities::log("Error on line " + intToStr(line_index) + ", file " + filename + ": invalid paragraph ID: " + invalid + ".\n");
				}
			}
		}
	}	file.close();
	ScriptUtilities::log("Link file read: " + filename + ".\n\n");
}

std::map<std::string, std::string> ScriptUtilities::parseLinkString(std::string& string) {
	std::map<std::string, std::string> ans;

	if (string.find('{') == string.npos)
		return ans;

	auto marks = split(split(string, '{')[1], ('}'))[0];
	for (auto mark : split(marks, ',')) {
		auto arr = split(mark, '=');
		ans[arr[0]] = arr[1];
	}

	string = cutMarks(string);
	return ans;
}
std::string ScriptUtilities::cutMarks(std::string string) {
	return split(string, '{')[0];
}

bool ScriptUtilities::isPlaying(){ return NARRATOR->bPlay; }

void ScriptUtilities::log(std::string text) {
	logOutput << text;
}

bool ParagraphConditionChecker::check(const ParagraphNode& node) { return false; }

Script::Script(std::string filename, int map_x, int map_y) {
	ScriptUtilities::script = this;
	graph = ScriptUtilities::readFile(filename);
	graph->play();
	bLoopPostcedence = true;
	map = new MapLayout(graph->getTiles(), map_x, map_y);

	setName("Script");

	skipController = new Controller(10);
	skipController->setToggleEvent(KEY_RELEASE, "Z", this, &Script::controllerZRelease);
	skipController->setName("skipCtrl");
	skipController->wakeUp();

	inventoryInfo = new GUI_Window(10, "empty_win", getwidth() - 60, 3, false, 30);
	inventoryInfo->setFunction("0", GUI_FUNCTION::STRDISPLAY, this, &Script::getInventory);
	inventoryInfo->formatNewestComponent();
	inventoryInfo->setNewestComponentColor(DWHITE);

	partyInfo = new GUI_Window(10, "empty_win", getwidth() - 30, 3, false, 30);
	partyInfo->setFunction("0", GUI_FUNCTION::STRDISPLAY, this, &Script::getParty);
	partyInfo->formatNewestComponent();
	partyInfo->setNewestComponentColor(DWHITE);

	for (auto it : getParagraph().tags)
		interpretTag(it);
}
void Script::destroy() {
	PrimitiveObject::destroy();
	inventoryInfo->destroy();
	delete map;
	delete graph;
	NARRATOR->hideWindow();
}
Script::~Script() { }

void Script::interpretTag(const std::string& tag) {
	if (tag == "scroll") {
		NARRATOR->scrollOpen();
	}	else
	if (tag == "maphide") {
		map->hide();
	}	else
	if (tag == "mapshow") {
		map->show();
	}	else
	if (tag == "gameover") {
		bOver = true;
	}	else 
	if (tag[0] == '{') NARRATOR->setAvatar(AvatarCollection::getAvatar(tag.substr(1)));
}

void Script::loop(float delta) {
	if (bDestroyed) return;
	PrimitiveObject::loop(delta);
	ParagraphNode& paragraph = getParagraph();

	if (!ScriptUtilities::isPlaying()) {
		int destination = -1;
		paragraph.endcode = NARRATOR->getSelection();
		for (auto& it : getLinks()) {
			if (it.second->check(paragraph)) {
				destination = it.first;
			}
		}

		if (destination != -1) {
			moveToNode(destination);
			if (map) map->travelTo(graph->getParagraph().mapTile);
			for (auto tag : getParagraph().tags)
				interpretTag(tag);
		}
	}
}


void Script::controllerZRelease() {
	if (bOver) return;
	while (true) {
		int destination = -1;
		ParagraphNode& paragraph = getParagraph();
		if (paragraph.options.size() != 0 || paragraph.promptText.size() != 0) {
			NARRATOR->skipText();
			NARRATOR->textAnimationProvider();
			break;
		}
		for (auto tag : paragraph.tags) {
			if (tag == "gameover") {
				bOver = true;
				break;
			}
		}
		if (bOver) break;
		paragraph.endcode = 0;
		for (auto& it : getLinks()) {
			if (it.second->check(paragraph)) {
				destination = it.first;
			}
		}

		if (destination != -1) {
			moveToNode(destination);
			if (map) map->travelTo(graph->getParagraph().mapTile);
			for (auto tag : getParagraph().tags)
				interpretTag(tag);
		}
		else break;
	}
}

ScriptGraph::ScriptGraph() {}
ScriptGraph::~ScriptGraph() {
	for (int i = 0; i < (int) paragraphs.size(); ++i) {
		for (auto it : edges[i]) {
			if (it.second) delete it.second;
		}
	}
}
void ScriptGraph::play() {
	auto& x = getParagraph();
	NARRATOR->playText(x.description);
	if (x.promptText.size() > 0)
		NARRATOR->setPrompt(x.promptText);
	for (auto& it : x.options) {
		int left = it.find('*');
		if (left != it.npos) {
			int right = it.find('*', left + 1);
			std::string tag = it.substr(left + 1, right - left - 1);
			if (ScriptUtilities::player->checkTag(tag))
				NARRATOR->addOption("x");
			else
				NARRATOR->addOption(it.substr(right + 1));
		}
		else {
			NARRATOR->addOption(it);
		}
	}
}

DemoScript::DemoScript() : Script("demo") {
	ScriptUtilities::readLinkFile("demo_links", graph);
	NARRATOR->showWindow();
	map->setRoomShape({ 0, 0 }, "startsquare_left");
	bOver = false;
	ending = "";

	for (int i = 0; i < EVENT_COUNT; ++i)
		eventFlagState[i] = false;

	graph->addEdge("enterchoice1",	"room2sec", new EndcodeValueChecker<3>(this, &DemoScript::checkFlag			<ROOM2>), true);
	graph->addEdge("enterchoice1",	"room2",	new EndcodeValueChecker<3>(this, &DemoScript::checkMinusFlag	<ROOM2>));
	graph->addEdge("enter2",		"room2sec", new EndcodeValueChecker<3>(this, &DemoScript::checkFlag			<ROOM2>), true);
	graph->addEdge("enter2",		"room2",	new EndcodeValueChecker<3>(this, &DemoScript::checkMinusFlag	<ROOM2>));
	graph->addEdge("room2sec",		"armor2",	new EndcodeValueChecker<1>(this, &DemoScript::checkFlag			<ARMOR>), true);
	graph->addEdge("room2sec",		"armor",	new EndcodeValueChecker<1>(this, &DemoScript::checkMinusFlag	<ARMOR>));
	graph->addEdge("room2sec",		"cellar2",	new EndcodeValueChecker<4>(this, &DemoScript::checkFlag			<CELLAR>), true);
	graph->addEdge("room2sec",		"cellar",	new EndcodeValueChecker<4>(this, &DemoScript::checkMinusFlag	<CELLAR>));

	PLAYERCH->addToInventory("guitar");
	PLAYERCH->addToInventory("dagger");
	PLAYERCH->addToInventory("torch");
	PLAYERCH->addToInventory("flint");
}

DemoScript::~DemoScript() {
}
void DemoScript::destroy() {
	Script::destroy();
}	

  
 bool Script::checkTag(std::string tag) {
	 return ScriptUtilities::player->tags.find(tag) != ScriptUtilities::player->tags.end();
 }
bool Script::checkMinusTag(std::string tag) {
	return ScriptUtilities::player->tags.find(tag) == ScriptUtilities::player->tags.end();
}

std::string Script::getInventory() {
	return PLAYERCH->getInventoryStringList();
}
std::string Script::getParty() {
	return PLAYERCH->getPartyStringList();
}

void DemoScript::interpretTag(const std::string& tag) {
	Script::interpretTag(tag);
	if (tag.substr(0, 3) == "xyz") {
		std::string add = tag.substr(3);
		ScriptUtilities::player->tags.insert(add);
	}
	else if (tag.substr(0, 3) == "zyx") {
		std::string add = tag.substr(3);
		ScriptUtilities::player->tags.erase(add);
	}
	else if (tag == "runaway") ending = "The Runaway";
	else if (tag == "mirrortag") eventFlagState[MIRROR] = true;
	else if (tag == "dining") eventFlagState[ROOM2] = true;
	else if (tag == "armor") eventFlagState[ARMOR] = true;
	else if (tag == "cellar") eventFlagState[CELLAR] = true;
}

void DemoScript::loop(float delta) {
	if (bDestroyed) return;
	Script::loop(delta);
}