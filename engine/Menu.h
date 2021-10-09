#ifndef GUI_MENU_H
#define GUI_MENU_H

#include "PrimitiveObject.h"
#include "Delegates.h"
#include "Debugger.h"

template <typename type> class PrimitiveClassMethodDelegate;
class Controller;
class GUI_Window;

struct Button {
	int x, y;
	GUI_Window *shape, *hoveredShape;
	bool bDelegated;
	PrimitiveClassMethodDelegate <void>* dlgEffect;
	void (*effect)();
};

class GUI_Menu : public PrimitiveObject {
public:
	GUI_Menu(int x = 0, int y = 0);
	GUI_Menu(std::string upButton, std::string downButton, int x = 0, int y = 0, bool bLoopable = false);
	GUI_Menu(std::vector <std::string> upButtons, std::vector <std::string> downButtons, int x = 0, int y = 0, bool bLoopable = false);

	virtual void destroy() override;

	void addButton(GUI_Window* sh, GUI_Window* hoverSh, int x, int y, void (*ptr)());
	template <typename type> void addButton(GUI_Window* sh, GUI_Window* hoverSh, int x, int y, type* obj, void (type::* ptr)());

	int buttonCount() { return (restriction < (int)buttons.size() ? restriction : (int)buttons.size()); }
	int restriction = 1000;

	void setPressLag(float value) { pressLag = value; }

	void freeze();
	void wakeUp();

	int hover;
	bool bAutoDestroy;
	bool bValid;

	void hide();
	void show();

protected:
	virtual ~GUI_Menu();

	virtual void loop(float delta) override;

	Controller *ctrl, *select;
	std::vector <Button> buttons;

	bool bHide;

	float pressLag;
	float transitionPast;
	float transitionTime;
	bool bLoopable, bTransition;
	bool bEscaped;
	int  x, y;

	virtual void pressHover();
	virtual void releaseHover();

	virtual void doTransition(float time) {}
	virtual void closeTransition() { bTransition = 0; }
	virtual void escape() { bEscaped = 1; }

private:
	inline void goUp()   { transitionPast = 0; bTransition = 1; hover = (hover == 0 ? (bLoopable ? buttonCount() - 1 : 0) : hover - 1); }; 
	inline void goDown() { transitionPast = 0; bTransition = 1; hover = (++hover == (int)buttonCount() ? (bLoopable ? 0 : buttonCount() - 1) : hover); };

	inline void press();

	friend class InputSingleSelector;
	friend class InputMultipleSelector;

	friend class Encounter;

	friend class Debugger;
};

template <typename type>
void GUI_Menu::addButton(GUI_Window* sh, GUI_Window* hoverSh, int x, int y, type* obj, void (type::* ptr)()) {
	buttons.push_back({ x, y, sh, hoverSh, 1 });
	buttons.back().dlgEffect = new ClassMethodDelegate <void, type>(obj, ptr);
}

class SelectorCell : public PrimitiveObject {
public:
	SelectorCell() { str = ""; }

	std::string str;
	std::string retrieveString() { return str; }

protected:
	virtual ~SelectorCell() {}

	friend class InputSingleSelector;
};

struct selector_settings {
	selector_settings(int offset = 0, int x_offset = 0, int height = 1, int width = 25, bool bAutoDestroy = true) {
		this->offset = offset, this->x_offset = x_offset, this->height = height, this->width = width, this->bAutoDestroy = bAutoDestroy;
	}	int offset, width, height;
	bool bAutoDestroy;
	int x_offset;
};

class InputSingleSelector : public PrimitiveObject {
public:
	InputSingleSelector(std::vector <std::string> choices, int x = 0, int y = 0, selector_settings settings = selector_settings());

	virtual void destroy() override { PrimitiveObject::destroy(); for (auto it : cells) it->destroy(); }

	std::string choice;
	void finalize();

	inline void resetChoice() { choice = ""; }
	std::string getHoveredChoice() { if (choices.empty() == false) return choices[menu->hover]; return ""; }

	GUI_Menu* menu;

protected:
	virtual ~InputSingleSelector() { }

	virtual void loop(float delta) override;

	std::vector <SelectorCell*> cells;
	std::vector <std::string> choices;

	selector_settings settings;

	friend class InputMultipleSelector;
	friend class PrimitiveAIBrain;

	friend class Debugger;
};

class InputMultipleSelector : public PrimitiveObject {
public:
	InputMultipleSelector(std::vector <std::string> choices, int count, int x = 0, int y = 0, bool bDistinct = true, selector_settings settings = selector_settings());
	std::vector <std::string> choices;

	virtual void destroy() override;

	int count;

	InputSingleSelector *selector() {
		if (choices.size() >= selectors.size()) return nullptr;
		return selectors[choices.size()];
	}

protected:
	virtual ~InputMultipleSelector() { }

	virtual void loop(float delta) override;

	selector_settings settings;
	std::vector <InputSingleSelector*> selectors;
	std::vector <std::string> targets;
	bool bDistinct;
	int  x, y;

	friend class Encounter;

	friend class Debugger;
};

#endif // GUI_MENU_H
