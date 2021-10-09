#ifndef CONSOLERENDERER_H
#define CONSOLERENDERER_H

#include "Colors.h"

#include <list>
#include <string>

class GUI_Window;
class VisualObject;
class Animator;
class ScreenObject;

class ConsoleRenderer
{
public:
	ConsoleRenderer();
	virtual ~ConsoleRenderer();

	virtual void addBackground(std::string name, bool bLoop = false, float speed = 1) = 0;
	virtual void resetBackground() = 0;
	virtual void addScrollAmount(float amount) = 0;

	virtual void addAnimator(Animator* anim) = 0;
	virtual void addGUI_Window(GUI_Window* win) = 0;
	virtual void addScreenObject(ScreenObject* obj) = 0;

	void overrideColor(COLOR color) {
		overrideValue = 1;
		colorReplace = color;
	}
	void resetOverride() {
		overrideValue = 0;
	}

protected:
	int overrideValue;
	int colorReplace;

	virtual void render(float delta) = 0;

private:

	friend class GodClass;

	friend class Debugger;
};

#endif // CONSOLERENDERER_H
