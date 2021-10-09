#include "Windows.h"

#include "ScreenObject.h"
#include "Controller.h"
#include "Timer.h"
#include "GodClass.h"
#include "Debugger.h"

#include <cstring>
#include <fstream>

#define BUFFER_SIZE 1005

int GUI_Window::objCount = 0;

FunctionalBox::FunctionalBox(int x1, int y1, int x2, int y2) : px1(x1), py1(y1), px2(x2), py2(y2) {
	color = Color();
	x = y = 0;
}


void GUI_Window::readFromFile(std::string wgui_name, int resize) {
	std::ifstream src(wgui_name);
	if (src) {
		src >> px >> py;
		int componentsCount; src >> componentsCount;
		std::string name;
		for (int i = 0; i < componentsCount; ++i)
			src >> name, componentsLink[name] = i;
		char str[BUFFER_SIZE]; src.get();
		int x = -px, y = -py;
		if (resize != -1) {
			data.clear();
			while (true) {
				x = -px;
				src.getline(str, BUFFER_SIZE);
				if (str[0] == '#') break;
				std::vector <int> stars;
				int len = strlen(str);
				if (resize < len) continue;
				for (int i = 0; i < len; ++i)
					if (str[i] == '*') stars.push_back(0);
				for (int i = 0; i < (int)stars.size(); ++i)
					stars[i] = (resize - len + 2 * stars.size() + stars.size() - (i + 1)) / stars.size();
				data.push_back({ y, x, "" });
				int idx = -1, bonus = 0;
				for (int i = 0, c = 0; i < len; ++i) {
					if ('0' <= str[i] && str[i] <= '9') {
						data.back().str += ' ';
						memComponentBounds[idx = (str[i] - '0')].push_back({ x + i + bonus, y });
					}
					else if (str[i] != '*') data.back().str += str[i];
					else {
						for (int j = 0; j < stars[c]; ++j)
							data.back().str += str[i + 1], ++bonus;
						++i; ++c;
					}
				}   ++y;
			}
		}
		else {
			data.clear();
			while (true) {
				x = -px;
				src.getline(str, BUFFER_SIZE);
				if (str[0] == '#') break;
				int len = strlen(str);
				data.push_back({ y, x, "" });
				int idx = -1, bonus = 0;
				for (int i = 0; i < len; ++i) {
					if ('0' <= str[i] && str[i] <= '9') {
						data.back().str += ' ';
						memComponentBounds[idx = (str[i] - '0')].push_back({ x + i + bonus, y });
					}
					else data.back().str += str[i];
				}   ++y;
			}
		}
	}   src.clear();
}

GUI_Window::GUI_Window(int priority, std::string wgui_name, int x, int y, bool bWorldSpace, int resize) : bWorldSpace(bWorldSpace) {
	this->renderPlane = priority;
	wgui_name = "resources/" + wgui_name + ".wgui";
	readFromFile(wgui_name, resize);
	setPosition(x, y);
	setName("GUI_Window");
	color = Color();
	GOD->getRenderer()->addGUI_Window(this);
	++ objCount;
}

GUI_Window::~GUI_Window() { -- objCount; }

void GUI_Window::reset() {
	GUI_components.clear();
	data.clear();
	for (int i = 0; i < 10; ++i) memComponentBounds[i].clear();
	componentsLink.clear();
	GUI_components.clear();
}

void GUI_Window::loop(float delta) {
	if (bDestroyed) return;
	VisualObject::loop(delta);
	setPosition(x, y);
}


GUI_WindowsTable::GUI_WindowsTable(std::string tgui_name, int resize) : px(0), py(0) {
	tgui_name = "resources/" + tgui_name + ".tgui";
	std::ifstream src(tgui_name);
	if (src) {
		src >> px >> py;
		int iterations = 0;
		while (true) {
			std::string str2; src >> str2;
			if (str2[0] == '#') break;
			src.get();
			link[str2] = iterations++;

			data.push_back(std::vector <OutputData>());
			for (int i = 0; i < 10; ++i) memComponentBounds[i].push_back(std::vector <std::pair <int, int>>());
			componentsLink.push_back(std::map <std::string, int>());

			int componentsCount; src >> componentsCount;
			std::string name;
			for (int i = 0; i < componentsCount; ++i)
				src >> name, componentsLink.back()[name] = i; src.get();
			char str[BUFFER_SIZE];
			int x = -px, y = -py;
			if (resize != -1) {
				while (true) {
					x = -px;
					src.getline(str, BUFFER_SIZE);
					if (str[0] == '#') break;
					std::vector <int> stars;
					int len = strlen(str);
					if (resize < len) continue;
					for (int i = 0; i < len; ++i)
						if (str[i] == '*') stars.push_back(0);
					for (int i = 0; i < (int)stars.size(); ++i)
						stars[i] = (resize - len + 2 * stars.size() + stars.size() - (i + 1)) / stars.size();
					data.back().push_back({ y, x, "" });
					int idx = -1, bonus = 0;
					for (int i = 0, c = 0; i < len; ++i) {
						if ('0' <= str[i] && str[i] <= '9') {
							data.back().back().str += ' ';
							memComponentBounds[idx = (str[i] - '0')].back().push_back({ x + i + bonus, y });
						}
						else if (str[i] != '*') data.back().back().str += str[i];
						else {
							for (int j = 0; j < stars[c]; ++j)
								data.back().back().str += str[i + 1], ++bonus;
							++i; ++c;
						}
					}   ++y;
				}
			}
			else {
				while (true) {
					x = -px;
					src.getline(str, BUFFER_SIZE);
					if (str[0] == '#') break;
					int len = strlen(str);
					data.back().push_back({ y, x, "" });
					int idx = -1, bonus = 0;
					for (int i = 0; i < len; ++i) {
						if ('0' <= str[i] && str[i] <= '9') {
							data.back().back().str += ' ';
							memComponentBounds[idx = (str[i] - '0')].back().push_back({ x + i + bonus, y });
						}
						else data.back().back().str += str[i];
					}   ++y;
				}
			}
		}
	}   src.close();
}

void GUI_WindowsTable::updateWindow(GUI_Window* win, int idx) {
	if (!win) return;
	win->reset();
	win->data = data[idx];
	win->px = px, win->py = py;
	for (int i = 0; i < 10; ++i) win->memComponentBounds[i] = memComponentBounds[i][idx];
	win->componentsLink = componentsLink[idx];
	win->data = data[idx];
}
void GUI_WindowsTable::updateWindow(GUI_Window* win, std::string nameLink) {
	if (link.find(nameLink) == link.end()) return;
	updateWindow(win, link[nameLink]);
}

PromptAsker::PromptAsker(Prompt* prompt, int minSize) {
	this->prompt = prompt;
	prompt->minSize = minSize;
	if (prompt->ctrl)
		prompt->ctrl->wakeUp();
	answer = "";
	bLoopPostcedence = 1;
	setName("asker");
}

bool PromptAsker::isPromptAlive() { return (prompt != nullptr && !prompt->wasDestroyed()); }
void PromptAsker::cancel() { prompt->destroy(); }

void PromptAsker::loop(float delta) {
	PrimitiveObject::loop(delta);
	if (prompt && !prompt->bDestroyed)
		answer = prompt->str;
}

Prompt::Prompt(std::string wgui_name, int x, int y, bool bWorldSpace, int resize) : GUI_Window(200, wgui_name, x, y, bWorldSpace, resize) {
	setFunction("0", STRDISPLAY, this, &Prompt::getString);
	setFunction("1", STRDISPLAY, this, &Prompt::getExplanation);
	explanation = "";
	ctrl = new AlphaController();
	ctrl->setName("alphaCtrl");
	setName("prompt");
	minSize = 0;
}

inline void Prompt::destroy() {
	GUI_Window::destroy();
	for (auto it : GUI_components) if (it) it->destroy();
	if (ctrl) ctrl->destroy();
}

Prompt::~Prompt() { }

void Prompt::loop(float delta) {
	if (bDestroyed) return;
	GUI_Window::loop(delta);
	if (!ctrl) { destroy(); return; }
	str = ctrl->str;

	int maxSize = 0;
	if (!GUI_components.empty() && GUI_components[0]) {
		auto it = GUI_components[0];
		maxSize = (it->px2 - it->px1 + 1) * (it->py2 - it->py1 + 1) - 3;
	}   while ((int)str.size() > maxSize) str.pop_back();
	ctrl->str = str;
	if (ctrl->bEntered) {
		if (str.size() >= minSize)
			enterData();
		else ctrl->bEntered = false;
	}
}

void Prompt::enterData() {
	destroy();
}

void Prompt::addCharacter(std::string key, char ch) {
	if (ctrl) ctrl->addCharacter(key, ch);
}

LoggerWindow::LoggerWindow(GUI_Window* win, int x, int y) {
	sign_x = -48; sign_y = 0;
	scrollMultiplier = 1;
	top = 0;
	bHidden = bOverflown = false;
	this->win = win;
	this->win->setPosition(0, 0);
	scrollSign = new ScreenObject(0, 0, 155);
	scrollSign->updateShape("        ^^^     SCROLLING... [UP KEY/DOWN KEY]    ");
	scrollSign->hide();
	scrollSign->setName("scroll");
	x1 = win->getFBoxPosition(0).first, y1 = win->getFBoxPosition(0).second;
	x2 = win->getFBoxPosition(1).first, y2 = win->getFBoxPosition(1).second;
	setPosition(x, y);
	hitTimer = nullptr;
}

LoggerWindow::~LoggerWindow() {  }

void LoggerWindow::destroy() {
	PrimitiveObject::destroy();
	scrollSign->destroy(); 
	win->destroy(); 
	for (auto it : paragraphs) it->destroy();
	if (hitTimer != nullptr) hitTimer->destroy();
}

void LoggerWindow::setPosition(int x, int y) { this->win->setPosition(x, y); 
	scrollSign->setPosition(x + (sign_x < 0 ? x2 : x1 ) + sign_x, y + (sign_y <= 0 ? y2 : y1) + sign_y); 
}

void LoggerWindow::addText(std::string str, Color textColor) {
	int lineLen = x2 - x1 + 1;	
	int idx = 0;
	std::vector <std::string> lines;
	lines.push_back(std::string());

	while (idx < (int)str.size()) {
		if ((idx == 0 || str[idx - 1] == ' ' || str[idx-1] == '-') && (str[idx] != ' ')) {
			int v = idx;
			while (v < (int)str.size() && (str[v] != ' ')) ++v;
			int len = v - idx;
			if (len > lineLen - (int)lines.back().size()) {
				while ((int)lines.back().size() != lineLen)
					lines.back() += ' ';
			}
		}

		if ((int)lines.back().size() == lineLen) {
			lines.push_back(std::string());
			while (idx < (int)str.size() && str[idx] == ' ') ++idx;
			if (idx == str.size()) break;
		}	lines.back() += str[idx++];
		if (idx == (int)str.size()) break;
	}  

	for (auto line : lines) {
		paragraphs.push_back(new ScreenObject(0, 0, 150)), paragraphs.back()->updateShape(line), paragraphs.back()->setName("pgph");
		paragraphs.back()->color = textColor;
	}
	if (bHidden) hide();
}
void LoggerWindow::clear() { for (auto it : paragraphs) it->destroy(); }

void LoggerWindow::scroll(int quantity) {
	if (!bOverflown || bHidden) return;
	quantity *= scrollMultiplier;
	int v = top + quantity;
	v = min((int)paragraphs.size()-y2+y1-1, max(v, 0));
	top = v;
}
void LoggerWindow::scrollUp() { scroll(1); }
void LoggerWindow::scrollDown() { scroll(-1); }

void LoggerWindow::hide() { bHidden = true; for (auto it : paragraphs) if (it) it->hide(); this->win->hide(); }
void LoggerWindow::show() { bHidden = false; for (auto it : paragraphs) if (it) it->show(); this->win->show(); }

void LoggerWindow::takeHit() {
	win->color = GRAY;
	if (hitTimer == nullptr || hitTimer->wasDestroyed()) {
		hitTimer = new Timer(0.2f, this, &LoggerWindow::resetHit);
	}
	else {
		hitTimer->resetTime();
	}
}

void LoggerWindow::resetHit() {
	win->color = WHITE;
}

void LoggerWindow::loop(float delta) {
	if (!scrollSign) destroy();
	if (bDestroyed) return;
	PrimitiveObject::loop(delta);
	if (bHidden) { scrollSign->hide(); return; }
	if ((int)paragraphs.size() >= y2 - y1 + 1) bOverflown = true;

	if (bOverflown) {
		if (top != 0) scrollSign->show();
		else		  scrollSign->hide();
		for (auto it : paragraphs) if (it) it->hide();
		int len = y2 - y1 + 1;
		int idx = paragraphs.size() - len - top;
		for (int i=0, idx = paragraphs.size() - len - top; i<len; ++i, ++idx)
			paragraphs[idx]->setPosition(win->getPosition().first + x1, win->getPosition().second + y1 + i), paragraphs[idx]->show();
	}	
	else {
		scrollSign->hide();
		for (int i=0; i<(int)paragraphs.size(); ++i)
			paragraphs[i]->setPosition(win->getPosition().first + x1, win->getPosition().second + y1 + i);
	}
}
