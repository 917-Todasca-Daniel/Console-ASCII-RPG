#include "Menu.h"

#include "Windows.h"
#include "Controller.h"
#include "Delegates.h"
#include "Timer.h"

GUI_Menu::GUI_Menu(int x, int y) : x(x), y(y) {
	bHide = bValid = false;
	transitionTime = 0;
	transitionPast = bTransition = bLoopable = bEscaped = false;
	pressLag = 0.001f;
	bLoopPostcedence = 1;
	hover = 0;
	ctrl = new Controller(0);
	ctrl->wakeUp();
	ctrl->setName("menuCtrl");
	select = new Controller(1);
	select->wakeUp();
	select->parent = this;
	select->setName("selCtrl");
	select->setToggleEvent(KEY_PRESS, "ENTER", this, &GUI_Menu::pressHover);
	select->setToggleEvent(KEY_RELEASE, "ENTER", this, &GUI_Menu::releaseHover);
	select->setToggleEvent(KEY_RELEASE, "ESCAPE", this, &GUI_Menu::escape);
	bAutoDestroy = true;
}

GUI_Menu::GUI_Menu(std::string upButton, std::string downButton, int x, int y, bool bLoopable) : GUI_Menu(x, y) {
	this->bLoopable = bLoopable;
	ctrl->setToggleEvent(KEY_PRESS, upButton, this, &GUI_Menu::goUp);
	ctrl->setToggleEvent(KEY_PRESS, downButton, this, &GUI_Menu::goDown);
	ctrl->parent = this;
	ctrl->wakeUp();
	bHide = false;
}
GUI_Menu::GUI_Menu(std::vector <std::string> upButtons, std::vector <std::string> downButtons, int x, int y, bool bLoopable) : GUI_Menu(x, y) {
	this->bLoopable = bLoopable;
	for (auto it : upButtons)
		ctrl->setToggleEvent(KEY_PRESS, it, this, &GUI_Menu::goUp);
	for (auto it : downButtons)
		ctrl->setToggleEvent(KEY_PRESS, it, this, &GUI_Menu::goDown);
	ctrl->parent = this;
	ctrl->wakeUp();
	bHide = false;
}

GUI_Menu::~GUI_Menu() {}

void GUI_Menu::destroy() {
	PrimitiveObject::destroy();
	for (auto it : buttons) {
		if (it.shape) it.shape->destroy();
		if (it.hoveredShape) it.hoveredShape->destroy();
	}
}

void GUI_Menu::pressHover() {
	if (ctrl) ctrl->freeze();
	bValid = true;
	bEscaped = 0;
}

void GUI_Menu::releaseHover() {
	if (!bValid) return;
	bValid = false;
	if (bEscaped) {
		if (ctrl) ctrl->wakeUp();
		return;
	}   if (select) select->freeze();
	new Timer(pressLag, this, &GUI_Menu::press);
}

void GUI_Menu::press() {
	if (!buttons.empty()) { if (buttons[hover].bDelegated) buttons[hover].dlgEffect->call(); else buttons[hover].effect(); }
	if (bAutoDestroy) destroy();
}

void GUI_Menu::loop(float delta) {
	PrimitiveObject::loop(delta);
	if (bTransition) {
		transitionPast += delta;
		if (transitionPast > transitionTime)
			closeTransition();
		else
			doTransition(transitionPast);
	}   if (bTransition) return;
	if (buttons.size() == 0) return;
	for (auto &it : buttons) if (it.shape && it.hoveredShape)
		it.hoveredShape->hide(), it.shape->show(),
		it.shape->setPosition(x + it.x, y + it.y),
		it.hoveredShape->setPosition(x + it.x, y + it.y);
	buttons[hover].hoveredShape->show();
	buttons[hover].shape->hide();

	if (bHide) {
		for (auto &it : buttons) {
			if (it.shape) it.shape->hide();
			if (it.hoveredShape) it.hoveredShape->hide();
		}
	}
}

void GUI_Menu::addButton(GUI_Window* sh, GUI_Window* hoverSh, int x, int y, void (*ptr)()) {
	buttons.push_back({ x, y, sh, hoverSh, 0 });
	buttons.back().effect = ptr;
}

void GUI_Menu::freeze() {
	//	TODO
	if (ctrl) ctrl->freeze();
	if (select) select->freeze();
}
void GUI_Menu::wakeUp() {
	if (ctrl) ctrl->wakeUp();
	if (select) select->wakeUp();
}

void GUI_Menu::hide() {
	bHide = true;
}
void GUI_Menu::show() {
	bHide = false;
}

InputSingleSelector::InputSingleSelector(std::vector <std::string> choices, int x, int y, selector_settings settings) : choices(choices) {
	if (choices.size() == 0) choices.push_back(" ");
	this->settings = settings;
	menu = new GUI_Menu("W", "S", x, y, true);
	menu->setName("selMenu");
	GUI_WindowsTable* table = new GUI_WindowsTable("battle_menu", 50);
	for (int i = 0; i < (int)choices.size(); ++i) {
		SelectorCell *ptr = new SelectorCell();
		ptr->setName("selCell");
		cells.push_back(ptr);
		cells.back()->str = choices[i];

		GUI_Window* win = new GUI_Window(106, "", 0, 0);
		win->setName("selBtn");
		table->updateWindow(win, 0);
		win->setFunction("0", STRDISPLAY, cells.back(), &SelectorCell::retrieveString);
		GUI_Window* hover_win = new GUI_Window(106, "", 0, 0);
		table->updateWindow(hover_win, 1);
		hover_win->setFunction("0", STRDISPLAY, cells.back(), &SelectorCell::retrieveString);
		hover_win->setName("selBtn");

		int x = settings.width * (i / settings.height) + (i % settings.height) * settings.x_offset;
		int y = settings.offset + (i % settings.height) * (1 + settings.offset);
		menu->addButton(win, hover_win, x, y, this, &InputSingleSelector::finalize);
		menu->bAutoDestroy = settings.bAutoDestroy;
	}	delete table;
	menu->wakeUp();
}

void InputSingleSelector::finalize() {
	if (menu->hover < (int)choices.size()) choice = choices[menu->hover];
	menu->wakeUp();
	if (settings.bAutoDestroy) menu->destroy();
}

void InputSingleSelector::loop(float delta) {
	PrimitiveObject::loop(delta);
}


InputMultipleSelector::InputMultipleSelector(std::vector<std::string> choices, int count, int x, int y, bool bDistinct, selector_settings settings) :
	targets(choices), x(x), y(y), bDistinct(bDistinct), count(count) {
	selectors.resize(count, nullptr); this->settings = settings; 
	selectors[0] = new InputSingleSelector(choices, x, y, settings);
	selectors[0]->setName("selector");
}

void InputMultipleSelector::destroy() {
	PrimitiveObject::destroy();
	if (choices.size() >= selectors.size()) return;
	auto& selector = selectors[choices.size()];
	if (selector) selector->destroy(), selector->menu->destroy();
	for (auto selector : selectors) if (selector) selector->destroy();
}

void InputMultipleSelector::loop(float delta) {
	PrimitiveObject::loop(delta);
	if (targets.size() == 0) while ((int)choices.size() < count) choices.push_back("-");
	if (choices.size() == count) return;
	auto& selector = selectors[choices.size()];
	if (selector == nullptr) {
		selector = new InputSingleSelector(targets, x, y, settings);
		selector->setName("selector");
	}
	if (selector->choice != "" && !selector->wasDestroyed()) {
		choices.push_back(selector->choice);
		if (bDistinct) targets.erase(std::remove(targets.begin(), targets.end(), selector->choice), targets.end());
		selector->destroy();
		selector->menu->destroy();
	}	
}
