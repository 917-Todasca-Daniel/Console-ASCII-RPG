#include "InmutableRenderer.h"

#include "Animator.h"
#include "ScreenObject.h"
#include "Windows.h"

#include "Colors.h"

#define	 DSU_ON	false

COLOR colorArray[] = { DARK, DBLUE, DGREEN, DCYAN, DRED, DPURPLE, DYELLOW, DWHITE, GRAY, BLUE, GREEN, CYAN, RED, PURPLE, YELLOW, WHITE };

InmutableRenderer::InmutableRenderer() : DSU(DisjointSetUnion(getwidth()*getheight())) { 
	prev = reset = { std::string(getwidth() * getheight(), ' '), std::vector <Color>(getwidth() * getheight(), Color(DWHITE)) };
}

InmutableRenderer::~InmutableRenderer() {
	for (auto it : requests) 
		delete it;
	for (auto it : waitingRequests) 
		delete it;
}

void InmutableRenderer::addAnimator(Animator* anim) {
	addRequest(new animationRequest(anim));
}
void InmutableRenderer::addGUI_Window(GUI_Window* win) {
	addRequest(new winRequest(win));
}
void InmutableRenderer::addScreenObject(ScreenObject* obj) {
	addRequest(new scbRequest(obj));
}

void InmutableRenderer::render(float delta) {
	manageRequests(delta);

	pres = reset;
	
	manageSegmentsOutput(delta);
	managePrint(delta);

	prev = pres;
}

void InmutableRenderer::manageSegmentsOutput(float delta) {
	if (DSU_ON) {
		DSU.reset();
		for (int i = (int)segments.size() - 1; i >= 0; --i) {
			int u = segments[i].x1;
			int v = segments[i].x2;

			int x = pres.output.size();
			if (segments[i].str.empty() || v < 0 || v >= (int)pres.output.size()) continue;
			
			v = DSU.labelSetMinimum(v);
			while (v >= u) {
				pres.output[v] = segments[i].str[v - segments[i].x1 + segments[i].idx];
				pres.colors[v] = segments[i].color;
				if (v > 0) {
					v = DSU.labelUnion(v, v - 1);
				}
				else {
					v = -1;
				}
			}
		}
	}
	else {
		for (auto& it : segments) {
			if (it.str.empty()) continue;
			for (int x = it.x1; x <= it.x2 && x < (int)pres.output.size(); ++x) {
				int idx = it.idx + x - it.x1;
				pres.output[x] = it.str[idx];
				pres.colors[x] = it.color;
			}
		}
	}
}

void InmutableRenderer::managePrint(float delta) {
	int idx = 0, pidx = -100, x = 0, y = 0, x2 = 0, y2 = 0;
	COLOR color = WHITE;
	std::string str = "";

	for (auto& it : pres.colors)
		it.forecolor = computeColor(it.forecolor);

	while (idx < (int)pres.output.size()) {
		if (pres.output[idx] == prev.output[idx] && pres.colors[idx].forecolor == prev.colors[idx].forecolor) {
			idx++;
		}
		else {
			if (idx - pidx < 30 && (pres.colors[idx].forecolor == color || pres.output[idx] == ' ')) {
				for (int j = pidx + 1; j <= idx; ++j) {
					if (pres.colors[j] != color && pres.output[j] != ' ') {
						j = idx;

						if (!str.empty()) {
							forcesetcursor(x2, y2);
							Color::setConsoleForecolor(color);
							coutput << str;
							str = "";
						}

						color = pres.colors[idx].forecolor;
						str += pres.output[idx];
						x2 = x;
						y2 = y;
					}
					else 
						str += pres.output[j];
				}
			}
			else {
				if (!str.empty()) {
					forcesetcursor(x2, y2);
					Color::setConsoleForecolor(color);
					coutput << str;
					str = "";
				}

				color = pres.colors[idx].forecolor;
				str += pres.output[idx];
				x2 = x;
				y2 = y;
			}

			pidx = idx;
			++idx;
		}

		if (++x == getwidth()) {
			++y;
			x = 0;
		}
	}

	if (!str.empty()) {
		forcesetcursor(x2, y2);
		Color::setConsoleColor(color);
		coutput << str;
	}
}

void InmutableRenderer::manageRequests(float delta) {
	auto it = requests.begin();
	while (it != requests.end()) {
		auto req = *it;
		if (req->isDestroyed()) {
			it = requests.erase(it);
			delete req;
			continue;
		}

		auto prev = req->order;
		req->update();
		if (req->order != prev) {
			it = requests.erase(it);
			waitingRequests.push_back(req);
		}
		else {
			++it;
		}
	}

	waitingRequests.sort(comparator());
	requests.merge(waitingRequests, comparator());
	waitingRequests.clear();

	segments.clear();
	for (auto it : requests)
		it->run(this);
}

void InmutableRenderer::addRequest(renderRequest* req) {
	waitingRequests.push_back(req);
}

void InmutableRenderer::addSegment(OutputData *data, Color color, VisualObject *obj, bool bPadding) {
	printSegment segment;
	int y = obj->y + data->y;
	int x1 = obj->x + data->x + bPadding*PADDING;
	int x2 = x1 + data->str.size() - 1;

	int idx = 0;

	if (x2 < bPadding*PADDING || x1 >= getwidth()) return;

	idx = bPadding * PADDING - x1;
	if (idx < 0) idx = 0;
	if (y < 0 || y >= getheight()) return;

	x1 = (std::max)(bPadding * PADDING, x1);
	x2 = (std::min)(getwidth() - 1, x2);

	segment.x1 = x1 + y*getwidth();
	segment.x2 = x2 + y*getwidth();

	segment.idx = idx;
	segment.color = color;
	segment.str = data->str;
	segments.push_back(segment);
}

InmutableRenderer::renderRequest::renderRequest(std::pair<int, int> order) : obj(nullptr), order(order) { }
InmutableRenderer::animationRequest::animationRequest(Animator* animation) : animation(animation), renderRequest({ animation->priority(), animation->render_order() }) {
	obj = animation;
}
InmutableRenderer::scbRequest::scbRequest            (ScreenObject* scb)   : scb(scb), renderRequest({ scb->priority(), scb->render_order() }) {
	obj = scb;
}
InmutableRenderer::winRequest::winRequest            (GUI_Window* win)     : win(win), renderRequest({ win->priority(), win->render_order() }) {
	obj = win;
}

void InmutableRenderer::animationRequest::run(InmutableRenderer *renderer) {
	if (animation->bHide) return;
	for (auto& it : animation->getFrame().data)
		renderer->addSegment(&it, animation->color, animation);
}
void InmutableRenderer::winRequest::run(InmutableRenderer* renderer) {
	if (win->bHide) return;
	for (auto& it : win->data)
		renderer->addSegment(&it, win->color, win, false);
}
void InmutableRenderer::scbRequest::run(InmutableRenderer* renderer) {
	if (scb->bHide) return;
	for (auto& it : scb->data)
		renderer->addSegment(&it, scb->color, scb);
}

void InmutableRenderer::renderRequest::update() {
	order = { obj->priority(), obj->render_order() };
}

bool InmutableRenderer::renderRequest::isDestroyed() {
	if (obj == nullptr || obj->wasDestroyed()) return true;
	return false;
}

void InmutableRenderer::addBackground(std::string name, bool bLoop, float speed) {
	std::ifstream file("resources/" + name + ".back");
	std::vector <std::string> lines;
	for (int i = 0; i < getheight(); ++i) {
		std::string line;
		std::getline(file, line);
		lines.push_back(line);
	}	backgrounds.push_back({ bLoop, 0, lines, speed, 0 });
	calculateBackgroundOutput();
}
void InmutableRenderer::calculateBackgroundOutput() {
	reset.output = "";
	int x = 0;
	int bgsize = backgrounds.size() - 1;
	for (int i = 0; i < getheight(); ++i) {
		for (int j = 0; j < getwidth() && x < getwidth() * getheight() - 1; ++j) {
			char ch = ' ';
			for (int bg = bgsize; bg >= 0; --bg) {
				auto& line = backgrounds[bg].lines[i];
				int idx = j + backgrounds[bg].idx;
				if (idx >= (int)line.size()) {
					if (!backgrounds[bg].bLoop) continue;
					int len = line.size();
					if (len == 0) continue;
					idx = (idx % len + len) % len;
				}
				if (line[idx] != ' ') {
					if (line[idx] == 9 || line[idx] == 11 || line[idx] == '5') ch = ' ';
					else ch = line[idx];
					break;
				}
			}
			reset.output += ch;
		}
	}
}

COLOR InmutableRenderer::computeColor(COLOR color) {
	return colorArray[color*(1-overrideValue) + overrideValue*colorReplace];
}

void InmutableRenderer::addScrollAmount(float amount) {
	bool bUpdate = false;
	for (auto& it : backgrounds) {
		it.scrollAmount += amount * it.scrollSpeed;
		if ((int)it.scrollAmount != it.idx)
			it.idx = (int)it.scrollAmount, bUpdate = true;;
	}	if (bUpdate) calculateBackgroundOutput();
}

void InmutableRenderer::resetBackground() {
	backgrounds.clear();
	reset.output = "";
	for (int i = 0; i < getwidth() * getheight() - 1; ++i)
		reset.output += " ";
}