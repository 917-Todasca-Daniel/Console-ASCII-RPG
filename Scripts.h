#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "engine/PrimitiveObject.h"
#include "engine/Delegates.h"

#include "engine/stdc++.h"

#include "fstream"

class ScreenObjectTable;

#define NARRATOR ScriptUtilities::narrator
#define PLAYERCH ScriptUtilities::player

template <typename type> class PrimitiveClassMethodDelegate;

class MapLayout {
public:
	template <typename container>
	MapLayout(const container &arr, int map_x, int map_y);
	~MapLayout();

	void setRoomShape(std::pair <int, int> pair, std::string shape);

	void travelTo(std::pair <int, int> pair);

	void hide();
	void show();
private:
	std::map <std::pair <int, int>, std::string> map;
	std::map <std::pair <int, int>, ScreenObject*> objMap;

	ScreenObjectTable* scbt;
};

struct ParagraphNode {
	std::pair <int, int> mapTile;
	int endcode;
	std::string description;
	std::string flag;
	std::vector <std::string> tags;
	std::vector <std::string> options;
	std::string promptText;
};

class ParagraphConditionChecker {
public:
	ParagraphConditionChecker() {}
	~ParagraphConditionChecker() {}

	virtual bool check(const ParagraphNode &node);
};
template <int value>
class EndcodeValueChecker : public ParagraphConditionChecker {
public:
	EndcodeValueChecker() { delegate = nullptr; bTagSearch = false; param_delegate = nullptr; tag = ""; }
	virtual ~EndcodeValueChecker() { if (delegate) delete delegate; }

	template <typename type>
	EndcodeValueChecker(type* obj, bool (type::* event)(void));

	template <typename type>
	EndcodeValueChecker(type* obj, bool (type::* event)(std::string), std::string tag);

	virtual bool check(const ParagraphNode& node);

protected:
	bool bTagSearch;
	std::string tag;

	PrimitiveClassMethodDelegate <bool> *delegate;
	PrimitiveStringClassMethodDelegate <bool> *param_delegate;
};

class ScriptGraph;
class Script;

class ScriptUtilities {
public:
	static ScriptGraph* readFile(std::string filename);
	static std::vector <std::string> parseOptions(std::string optionsline);
	static void readLinkFile(std::string filename, ScriptGraph*& graph);

	static std::map <std::string, std::string> parseLinkString(std::string &string);
	static std::string cutMarks(std::string string);

	static bool isPlaying();

	static Script* script;

	static void log(std::string text);

	static std::ofstream logOutput;

	static class Player* player;
	static class Narration* narrator;
};

class ScriptGraph {
public:
	ScriptGraph();
	virtual ~ScriptGraph();

	ParagraphNode& getParagraph() { return paragraphs[index]; }
	std::vector <std::pair <int, ParagraphConditionChecker*>> getLinks() { return edges[index]; }

	void addNode(ParagraphNode node) {
		int n = paragraphs.size();
		flagMap[node.flag] = n;
		
		if (n > 0) edges[n - 1].push_back({ n, new EndcodeValueChecker <0>() });
		paragraphs.push_back(node);
		edges.push_back(std::vector <std::pair <int, ParagraphConditionChecker*>>());
	}
	void addEdge(int idx, int idy, ParagraphConditionChecker* checker, bool bClear = false) {
		if (bClear) {
			for (int i = 0; i < (int)edges[idx].size(); ++i)
				if (edges[idx][i].first == idy) {
					edges[idx].erase(edges[idx].begin() + i);
					break;
				}
		}

		edges[idx].push_back({ idy, checker });
	}
	void addEdge(std::string from, std::string to, ParagraphConditionChecker* checker, bool bClear = false) {
		if (flagMap.find(from) == flagMap.end()) {
			throw from;
		}
		if (flagMap.find(to) == flagMap.end()) {
			throw to;
		}
		addEdge(flagMap[from], flagMap[to], checker, bClear);
	}

	void checkParentReferenceValidity(std::string parent, int num) {
		if (flagMap.find(parent) == flagMap.end()) {
			throw parent;
		}

		auto p = paragraphs[flagMap[parent]];
		if (p.promptText == "" && p.options.empty()) {
			ScriptUtilities::log("Bad reference for paragraph with name: " + parent + " (cannot be referenced).\n");
		}
		else {
			int expected = p.options.size();
			if (p.promptText != "") {
				expected = split(p.promptText, '*').size()-1;
			}

			if (expected != num)
				ScriptUtilities::log("Bad reference for paragraph with name: " + parent + " (bad number of references): " + intToStr(num) + ", " + intToStr(expected) + ".\n");
		}
	}

	int getIndex() { return index; }

	void moveToNode(int idx) { index = idx; }
	void play();

	std::set <std::pair <int, int>> getTiles() { return tiles; }

protected:
	int index = 0;
	std::vector <ParagraphNode> paragraphs;
	std::vector <std::vector <std::pair <int, ParagraphConditionChecker*>>> edges;
	std::map <std::string, int> flagMap;

	std::set <std::pair <int, int>> tiles;

	friend class Debugger;
	friend class ScriptUtilities;
};

class Script : public PrimitiveObject {
public:
	Script(std::string filename, int map_x = 10, int map_y = 10);

	virtual void destroy() override;

	inline bool isOver() { return bOver; }

	inline void hideMap() { map->hide(); }

	bool checkTag(std::string tag);
	bool checkMinusTag(std::string tag);

	std::string getInventory();
	std::string getParty();

protected:
	virtual ~Script();

	virtual void interpretTag(const std::string &tag);

	virtual void loop(float delta) override;

	bool bOver;

	void controllerZRelease();

	Controller *skipController;
	ScriptGraph *graph;
	MapLayout* map;
	GUI_Window* inventoryInfo;
	GUI_Window* partyInfo;

	std::vector <std::pair <int, ParagraphConditionChecker*>> getLinks() { return graph->getLinks(); }
	ParagraphNode& getParagraph() { return graph->getParagraph(); }
	void moveToNode(int idx) { graph->moveToNode(idx); graph->play(); }

	friend class Debugger;
};

enum DEMO_EVENT_FLAGS { ARMOR, MIRROR, ROOM2, CELLAR, EVENT_COUNT};

class DemoScript : public Script {
public:
	DemoScript();
	
	virtual void destroy() override;
	std::string getEnding() { return ending; }

	template <DEMO_EVENT_FLAGS flag>
	bool checkFlag() {
		return eventFlagState[flag];
	}

	template <DEMO_EVENT_FLAGS flag>
	bool checkMinusFlag() {
		return !eventFlagState[flag];
	}

protected:
	virtual ~DemoScript();

	std::string ending;

	virtual void interpretTag(const std::string& tag) override;

	virtual void loop(float delta) override;

	bool eventFlagState[EVENT_COUNT];

	friend class Debugger;
};

template<int value>
inline bool EndcodeValueChecker<value>::check(const ParagraphNode& node) {
	if (param_delegate && !param_delegate->call(tag)) return false;
	if (delegate && !delegate->call()) return false;
	return node.endcode == value;
}

#endif // SCRIPTS_H

template<int value>
template<typename type>
inline EndcodeValueChecker<value>::EndcodeValueChecker(type* obj, bool (type::* event)(void)) {
	delegate = new ClassMethodDelegate <bool, type>(obj, event);
	bTagSearch = false;
}

template<int value>
template<typename type>
inline EndcodeValueChecker<value>::EndcodeValueChecker(type* obj, bool(type::* event)(std::string), std::string tag) {
	bTagSearch = true;
	this->tag = tag;
	param_delegate = new ParameteredStringClassMethodDelegate <bool, type>(obj, event);
}
