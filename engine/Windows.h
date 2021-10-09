#ifndef GUI_WINDOWS_H
#define GUI_WINDOWS_H

#include "VisualObject.h"
#include "GarbageTruck.h"
#include "Delegates.h"
#include "Debugger.h"
#include "Colors.h"
#include "ScreenObject.h"

#include <utility>
#include <vector>
#include <map>

enum GUI_FUNCTION { STRDISPLAY = 0, OBJDISPLAY };

class ScreenObject;
class Controller;
class AlphaController;
template <typename type>
class FadeBehaviour;
template <typename type>
class ScrollAppearance;
class Timer;

class FunctionalBox : public PrimitiveObject {
public:
	FunctionalBox(int x1, int y1, int x2, int y2);

	virtual inline void setPosition(int x, int y) { this->x = x, this->y = y; }

	Color color;

	virtual void show() = 0;
	virtual void hide() = 0;

protected:
	int px1, py1, px2, py2;
	int x, y;

	friend class GUI_Window;
	friend class Prompt;

	friend class Debugger;
};

template <typename type> class DisplayBox : public FunctionalBox {
public:
	DisplayBox(int x1, int y1, int x2, int y2, int priority, type(*provider)());
	DisplayBox(int x1, int y1, int x2, int y2, int priority, PrimitiveClassMethodDelegate <type>* delegate);

	virtual inline void setPosition(int x, int y) override;

	virtual void hide() override { displayObject->hide(); };
	virtual void show() override { displayObject->show(); };
	bool bFormat;

protected:
	virtual ~DisplayBox();

	ScreenObject* displayObject;
	std::vector <struct OutputData> boundsPos;
	std::vector <int>               boundsLen;

	bool bDelegated;
	union {
		type(*strprovider)();
		PrimitiveClassMethodDelegate <type>* delegate;
	};

	virtual void destroy() override;

private:
	virtual void loop(float delta) override;

	friend class Debugger;
};

class GUI_Window : public VisualObject {
public:
	GUI_Window(int priority, std::string wgui_name, int x, int y, bool bWorldSpace = 0, int resize = -1);

	inline virtual void destroy() override { bDestroyed = 1; for (auto it : GUI_components) if (it && !it->bDestroyed) it->destroy(); }

	template <typename cb_type>
	void setFunction(std::string functionalBoxName, GUI_FUNCTION functionality, cb_type* callback, Color color = Color());

	template <typename callback_return, typename classtype>
	void setFunction(std::string functionalBoxName, GUI_FUNCTION functionality, classtype* obj, callback_return(classtype::* func)(), Color color = Color());

	inline virtual void hide() override { VisualObject::hide(); for (auto& it : GUI_components) it->hide(); };
	inline virtual void show() override { VisualObject::show(); for (auto& it : GUI_components) it->show(); };

	inline void setPosition(int x, int y) override {
		this->x = x, this->y = y; 
		for (auto& it : GUI_components) it->setPosition(x, y); 
	}

	void setComponentColor(int idx, Color color) { if (!GUI_components.empty()) GUI_components[idx]->color = color; }
	void setNewestComponentColor(Color color)    { setComponentColor(GUI_components.size()-1, color); }

	void formatNewestComponent() {
		DisplayBox <std::string>* cast = dynamic_cast <DisplayBox <std::string>*> (GUI_components.back());
		if (cast) {
			cast->bFormat = true;
		}
	}
	void clearComponents() { 
		for (auto it : GUI_components) if (it) it->destroy(); 
		GUI_components.clear(); 
	}
	void forceFormat() {
		for (auto it : GUI_components) {
			DisplayBox <std::string>* display = dynamic_cast <DisplayBox <std::string>*> (it);
			if (display) display->bFormat = 1;
		} 
	}

	std::pair <int, int> getFBoxPosition(int idx) { return memComponentBounds[idx].front(); }
	void setFboxColor(int idx, Color colour) { GUI_components[idx]->color = colour; }

	template <typename Functor>
	void cut(Functor condition) {
		VisualObject::cut(condition, data);
	}

protected:
	virtual ~GUI_Window();

	virtual void loop(float delta) override;

	std::vector <OutputData> data;

	bool bWorldSpace;
	int  px, py;
	std::vector <FunctionalBox*>       GUI_components;
	std::map    <std::string, int>     componentsLink;
	std::vector <std::pair <int, int>> memComponentBounds[10];

	void reset();

private:
	static int objCount;

	void readFromFile(std::string wgui_name, int resize);
	int  newest;


	friend class InmutableRenderer;
	friend class ConsoleRenderer;
	
	friend class FadeBehaviour <GUI_Window>;
	friend class ScrollAppearance <GUI_Window>;
	friend class GUI_WindowsTable;
	friend class GUI_Menu;

	friend class Debugger;
};

class GUI_WindowsTable : public GarbageTruck {
public:
	GUI_WindowsTable(std::string tgui_name, int resize = -1);

	void updateWindow(GUI_Window* win, int idx);
	void updateWindow(GUI_Window* win, std::string nameLink);

protected:
	int px, py;

	std::vector <std::map    <std::string, int>>     componentsLink;
	std::vector <std::vector <std::pair <int, int>>> memComponentBounds[10];
	std::vector <std::vector <OutputData>> data;
	std::map <std::string, int> link;

	friend class Debugger;
};

class LoggerWindow : public PrimitiveObject {
public:
	LoggerWindow(GUI_Window* win, int x = 0, int y = 0);

	virtual void destroy() override;

	void setPosition(int x, int y);

	void addText(std::string str, Color textColor);
	void clear();

	void resetScroll() { scroll(-1000000); }
	
	void scroll(int quantity);
	void scrollUp();
	void scrollDown();

	virtual void hide();
	virtual void show();

	virtual void takeHit();
	virtual void resetHit();

	bool bHidden;
	bool bOverflown;

	int scrollMultiplier;

protected:
	virtual ~LoggerWindow();

	virtual void loop(float delta) override;

	ScreenObject *scrollSign;
	std::vector <ScreenObject*> paragraphs;
	int top;

	Timer *hitTimer;

	GUI_Window *win;
	int x1, y1, x2, y2;
	int sign_x, sign_y;

	friend class Debugger;
};

class Prompt;
class PromptAsker;
class Prompt : public GUI_Window {
public:
	Prompt(std::string wgui_name, int x, int y, bool bWorldSpace = 0, int resize = -1);

	inline virtual void destroy() override;

	virtual std::string getString() { return "> " + str + "_"; }
	void addCharacter(std::string key, char ch);

	void write(std::string str) { explanation = str; }

	std::string getExplanation() { return explanation; }

protected:
	virtual ~Prompt();

	std::string      str;
	std::string		 explanation;
	AlphaController* ctrl;
	int minSize;

	virtual void enterData();

	virtual void loop(float delta) override;

	friend class Debugger;
	friend class PromptAsker;
};

class PromptAsker : public PrimitiveObject {
public:
	PromptAsker(Prompt* prompt, int minSize = 0);
	std::string answer;

	bool isPromptAlive();

	void cancel();
	void write(std::string str) { prompt->write(str); }

protected:
	Prompt* prompt;

	virtual void loop(float delta) override;

	friend class Debugger;
};


template <typename type, typename type2> static FunctionalBox* maker(int x1, int y1, int x2, int y2, int priority, type2 t) {
	return new type(x1, y1, x2, y2, priority, t);
}

static void* const makers[] = { (void*)maker <DisplayBox <std::string>, std::string(*)()>,
	(void*)maker <DisplayBox <ScreenObject*>, ScreenObject*(*)()> };
static void* const makers2[] = { (void*)maker <DisplayBox <std::string>, PrimitiveClassMethodDelegate <std::string>*>, 
	(void*)maker <DisplayBox <ScreenObject*>, PrimitiveClassMethodDelegate <ScreenObject*>*> };

template <typename cb_type> void GUI_Window::setFunction(std::string functionalBoxName, GUI_FUNCTION functionality, cb_type* callback, Color color) {
	if (componentsLink.find(functionalBoxName) == componentsLink.end()) return;
	int idx = componentsLink[functionalBoxName]; 
	std::vector <std::pair <int, int>>& it = memComponentBounds[idx];
	GUI_components.push_back(reinterpret_cast <FunctionalBox * (*)(int, int, int, int, int, cb_type)> (makers[functionality])(it[0].first, it[0].second, it[1].first-2, it[1].second, renderPlane+1, callback));
	GUI_components.back()->setPosition(x, y);
	GUI_components.back()->bLoopPostcedence = 1;
	GUI_components.back()->color.setColor(color);
}

template <typename callback_return, typename classtype> void GUI_Window::setFunction(std::string functionalBoxName, GUI_FUNCTION functionality, classtype* obj, callback_return(classtype::* func)(), Color color) {
	if (componentsLink.find(functionalBoxName) == componentsLink.end()) return;
	int idx = componentsLink[functionalBoxName];
	std::vector <std::pair <int, int>>& it = memComponentBounds[idx];
	PrimitiveClassMethodDelegate <callback_return>* delegate = new ClassMethodDelegate <callback_return, classtype>(obj, func);
	if (!delegate) return;
	GUI_components.push_back(reinterpret_cast <FunctionalBox * (*)(int, int, int, int, int, PrimitiveClassMethodDelegate <callback_return>*)>
		(makers2[functionality])(it[0].first, it[0].second, it[1].first-2, it[1].second, renderPlane+1, delegate));
	GUI_components.back()->setPosition(x, y);
	GUI_components.back()->bLoopPostcedence = 1;
	GUI_components.back()->color.setColor(color);
}

template <typename type> void DisplayBox <type>::destroy() {
	PrimitiveObject::destroy();
	if (displayObject) displayObject->destroy();
}
template <typename type> void DisplayBox <type>::setPosition(int x, int y) {
	FunctionalBox::setPosition(x, y);
	if (displayObject) displayObject->setPosition(x, y);
}

template <typename type> DisplayBox <type>::DisplayBox(int x1, int y1, int x2, int y2, int priority, type(*provider)()) : FunctionalBox(x1, y1, x2, y2) {
	this->displayObject = new ScreenObject(px1, py1, priority);
	if (this->displayObject)
		this->displayObject->bOverrideInnateRules = 1;
	for (int y = y1; y <= y2; ++y) boundsPos.push_back({ y - y1, 0, "" }), boundsLen.push_back(x2 - x1 + 1);
	this->strprovider = provider;
	bDelegated = 0;
	setName("Display");
	bFormat = 0;
}

template <typename type> DisplayBox <type>::DisplayBox(int x1, int y1, int x2, int y2, int priority, PrimitiveClassMethodDelegate <type>* delegate) : FunctionalBox(x1, y1, x2, y2) {
	this->displayObject = new ScreenObject(px1, py1, priority);
	if (this->displayObject)
		this->displayObject->bOverrideInnateRules = 1;
	for (int y = y1; y <= y2; ++y) boundsPos.push_back({ y - y1, 0, "" }), boundsLen.push_back(x2 - x1 + 1);
	this->delegate = delegate;
	bDelegated = 1;
	setName("Display");
	bFormat = 0;
}

template <typename type> DisplayBox <type>::~DisplayBox() { if (bDelegated) delete delegate; }

template <typename type> void DisplayBox <type>::loop(float delta) {
	if (bDestroyed) return;
	PrimitiveObject::loop(delta);
	if (displayObject && !displayObject->wasDestroyed()) {
		if (!bDelegated) displayObject->updateShape(strprovider(), boundsPos, boundsLen, bFormat);
		else displayObject->updateShape(delegate->call(), boundsPos, boundsLen, bFormat);
		displayObject->color.setColor(color);
		displayObject->setPosition(px1 + x - PADDING * (1 - displayObject->bOverrideInnateRules), py1 + y);
	}
}

#endif // GUI_WINDOWS_H
