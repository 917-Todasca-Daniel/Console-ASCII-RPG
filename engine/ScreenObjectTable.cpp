#include "ScreenObjectTable.h"

#include "ScreenObject.h"

#include <fstream>

#define BUFFER_SIZE 1005

ScreenObjectTable::ScreenObjectTable(std::string tscb_name) {
	tscb_name = "resources/" + tscb_name + ".tscb";
	std::ifstream src(tscb_name);
	if (src) {
		int px, py; src >> px >> py;
		src.get();
		while (true) {
			std::string str2; src >> str2; if (str2[0] == '#') break; int idx = link.size(); link[str2] = idx;
			shapes.push_back(ScreenObject::readOutputDataFromFile(src));
			for (auto& it : shapes.back())
				it.x -= px, it.y -= py;
		}   src.close();
	}
}

ScreenObjectTable::~ScreenObjectTable() {}

void ScreenObjectTable::updateShape(ScreenObject *obj, int tableIndex) {
	if (!obj) return;
	obj->data = shapes[tableIndex];
}
void ScreenObjectTable::updateShape(ScreenObject *obj, std::string tableName) {
	if (link.find(tableName) == link.end()) return;
	updateShape(obj, link[tableName]);
}

