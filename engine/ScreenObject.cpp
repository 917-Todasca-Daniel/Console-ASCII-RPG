#include "ScreenObject.h"

#include "GodClass.h"
#include "ScreenObjectTable.h"

#include "consolewizard.h"

#define BUFFER_SIZE 1005

int ScreenObject::objCount = 0;

ScreenObject::ScreenObject(int x, int y, int priority, std::string scb_name) {
	show();
	this->x = x, this->y = y, this->renderPlane = priority;
	if (scb_name != "") {
		scb_name = "resources/" + scb_name + ".scb"; std::ifstream src(scb_name);
		updateShape(src);
	}   setName("ScreenObj");
	color = Color();
	GOD->getRenderer()->addScreenObject(this);
	++ objCount;
}

ScreenObject::ScreenObject(int x, int y, int priority, ScreenObjectTable *table, std::string tableName) {
	show();
	this->x = x, this->y = y, this->renderPlane = priority;
	if (table) {
		table->updateShape(this, tableName);
	}   setName("ScreenObj");
	color = Color();
	GOD->getRenderer()->addScreenObject(this);
	++ objCount;
}
ScreenObject::ScreenObject(int x, int y, int priority, ScreenObjectTable *table, int tableIdx) {
	show();
	this->x = x, this->y = y, this->renderPlane = priority;
	if (table) {
		table->updateShape(this, tableIdx);
	}   setName("ScreenObj");
	color = Color();
	GOD->getRenderer()->addScreenObject(this);
	++ objCount;
}


ScreenObject::~ScreenObject() { -- objCount; }
void ScreenObject::destroy() {
	VisualObject::destroy();
}

void ScreenObject::loop(float delta) {
	VisualObject::loop(delta);
}

void ScreenObject::updateShape(std::string str) {
	show();
	data.clear();
	data.push_back({ 0, 0, str });
}

void ScreenObject::updateShape(std::string fill, std::vector <OutputData> shape, std::vector <int> lens, bool bFormat) {
	data.clear();
	int idx0 = 0, idx2 = 0;

	while (idx0 < (int)fill.size()) {
		if (bFormat) {
			if ((idx0 == 0 || fill[idx0 - 1] == ' ') && fill[idx0] != ' ') {
				int v = idx0;
				while (v < (int)fill.size() && fill[v] != ' ') ++v;
				int len = v - idx0;
				if (len > lens[idx2] - (int)shape[idx2].str.size()) {
					if (++idx2 == (int)shape.size()) break;
				}
			}
		}

		if (fill[idx0] == '\n' || (int)shape[idx2].str.size() == lens[idx2]) {
			if (++idx2 == (int)shape.size()) break;
			if (bFormat) {
				while (idx0 < (int)fill.size() && fill[idx0] == ' ') ++idx0;
				if (idx0 == fill.size()) break;
			}
		}
		if (fill[idx0] != '\n')
			shape[idx2].str += fill[idx0++];
		else ++ idx0;
		if (idx0 == (int)fill.size()) break;
	}   for (auto it : shape) data.push_back(it);
}

void ScreenObject::updateShape(std::ifstream& src) {
	data.clear();
	int px, py; src >> px >> py;
	src.get();
	data = VisualObject::readOutputDataFromFile(src);
	for (auto& it : data)
		it.x -= px, it.y -= py;
	src.close();
}

void ScreenObject::updateShape(ScreenObjectTable *table, int tableIndex) {
	table->updateShape(this, tableIndex);
}

void ScreenObject::updateShape(ScreenObject *obj, std::vector <OutputData> shape, std::vector <int> lens, bool bFormat) {
	if (obj == nullptr) {
		updateShape("");
		return;
	}
	this->data = obj->data;
}

void ScreenObject::updateShape(ScreenObjectTable *table, std::string tableName) {
	table->updateShape(this, tableName);
}

int GridTile::render_order() {  return (getwidth() - x) + (getheight() - y) * getwidth(); }

void TextFloatingPopup::loop(float delta) {
	ScreenObject::loop(delta);
	if (lifespan > maxtime) {
		destroy();
		hide();
	}
	else {
		amount += speed*delta;
		setPosition(orx, (int)(ory - amount));
	}
}
