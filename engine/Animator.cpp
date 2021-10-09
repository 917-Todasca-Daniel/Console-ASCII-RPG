#include "Animator.h"

#include "AnimatorTable.h"
#include "GodClass.h"
#include "Colors.h"

#include <fstream>
#include <sstream>

#define BUFFER_SIZE 105

float Animator::globalSpeed = 1.0f;
int Animator::objCount = 0;

void Animator::getDataFromFile(std::string asc_name) {
	std::ifstream src(asc_name);
	if (src) {
		int px, py; src >> px >> py;
		char str[BUFFER_SIZE];
		src.get(), src.getline(str, BUFFER_SIZE);
		int num;
		std::stringstream ss(str);
		while (!ss.eof() && ss >> num)
			frames.push_back({ num, std::vector <OutputData>() });

		for (int i = 0; i < (int)frames.size(); ++i) {
			frames[i].data = readOutputDataFromFile(src);
			for (auto& it : frames[i].data)
				it.x -= px, it.y -= py;
		}   src.close();
	}
}

Animator::Animator(std::string asc_name, int renderPlane, bool bPlay, bool fireAndForget) {
	this->renderPlane = renderPlane;
	asc_name = "resources/" + asc_name + ".asc";
	reset();
	bPlaying = bPlay;
	getDataFromFile(asc_name);
	setName("Animator");
	color = Color();
	startFrame = 0;
	this->fireAndForget = fireAndForget;
	GOD->getRenderer()->addAnimator(this);
	++objCount;
}
Animator::Animator(std::string asc_name, int x, int y, int renderPlane, bool bPlay, bool fireAndForget) : Animator(asc_name, renderPlane, bPlay, fireAndForget) {
	this->x = x;
	this->y = y;
	GOD->getRenderer()->addAnimator(this);
	++objCount;
}

Animator::Animator(AnimatorTable *table, int tableIndex, int x, int y, int renderPlane, bool bPlay, bool fireAndForget) {
	this->renderPlane = renderPlane;
	this->x = x;
	this->y = y;
	reset();
	bPlaying = bPlay;
	if (table) table->animate(this, tableIndex);
	setName("Animator");
	color = Color();
	this->fireAndForget = fireAndForget;
	GOD->getRenderer()->addAnimator(this);
	++objCount;
}
Animator::Animator(AnimatorTable *table, std::string tableName, int x, int y, int renderPlane, bool bPlay, bool fireAndForget) {
	this->renderPlane = renderPlane;
	this->x = x;
	this->y = y;
	reset();
	bPlaying = bPlay;
	if (table) table->animate(this, tableName);
	setName("Animator");
	color = Color();
	this->fireAndForget = fireAndForget;
	GOD->getRenderer()->addAnimator(this);
	++objCount;
}

Animator::~Animator() { --objCount; }
void Animator::destroy() {
	VisualObject::destroy();
}

void Animator::loop(float delta) {
	VisualObject::loop(delta);
	if (isPlaying()) timer += delta * speed * globalSpeed;
	if (!isPlaying()) return;
	while (isFrameOver())
		currentFrame = advanceFrame();
}

void Animator::updateAnimation(AnimatorTable* table, std::string str) {
	table->animate(this, str);
}

void Animator::flipX(int xAxis) {
	for (auto& it : frames) {
		VisualObject::flipX(it.data, xAxis);
	}
}

