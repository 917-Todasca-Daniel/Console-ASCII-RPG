#ifndef NARRATION_H
#define NARRATION_H

#include "engine/PrimitiveObject.h"

#include <string>
#include <vector>

class GUI_Window;
class ScreenObject;
class GUI_Menu;
class GUI_WindowsTable;
class Timer;
class PromptAsker;

class Narration : public PrimitiveObject {
public:
	Narration(int x, int y, int resize, bool bMenu = true, std::string filename = "narratorbox");

	virtual void destroy() override;

	void playText (std::string text);
	void addOption(std::string text);

	void setPrompt(std::string text = "What will you do?");

	void		  setAvatar(ScreenObject* obj) { avatar = obj; }
	ScreenObject* getAvatar()				   { return avatar; }

	void disableSkip();

	void freeze();
	void wakeUp();

	std::string textAnimationProvider();
	std::string getExplanationText() { return explanation; }
	int getSelection() { 
		if (optionMap.empty()) return selection;
		return optionMap[selection]; 
	}

	void hideWindow();
	void showWindow();

	void scrollOpen();

	void skipText();
	void controllerEnterRelease();

	template <int num> std::string getOption();
	void menuButtonPress();

	bool bPlay;
	bool bEntered = false;

private:
	virtual ~Narration();

	virtual void loop(float delta) override;

	void addBlankButton(int x, int y, std::string(Narration::* f1)(), std::string(Narration::* f2)(), COLOR unhovered, COLOR hovered);

	PromptAsker* createPrompt();

	Controller*		  ctrl;
	ScreenObject*	  avatar;
	GUI_Window*		  win;
	GUI_Menu*		  menu;
	GUI_WindowsTable* table;

	std::map <int, int> optionMap;

	int   selection = -1;
	int   optionOffset;
	float time = 0;
	int   idx  = 0;
	int	  noptions = 0;
	float prevchars = 0;
	float promptRefresh = 0;

	bool  bFreeze;
	bool  bSkip;

	float charPerSecond = 50;
	float delay			= 10;
	float bonus			= 20;
	std::string pauseSigns  = ",?!.";
	std::string strongSigns = "?!.";

	std::vector <std::string> matches;
	std::string options[4];
	std::string explanation;
	std::string text;

	bool bPrompt;
	std::string promptText;

	PromptAsker *prompt;

	friend class Debugger;
};

template<int num>
inline std::string Narration::getOption() {
	if (idx+1 >= (int)text.size()) return options[num-1];
	return "";
}

#endif // NARRATION_H