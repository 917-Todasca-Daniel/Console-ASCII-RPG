#include "VisualObject.h"

#include "consolewizard.h"

#define BUFFER_SIZE 1005

std::map <char, char> VisualObject::flipRules = { {'>', '<'}, {'\\', '/'}, {'/', '\\'}, {'(', ')'}, {')', '('}, {'[', ']'}, {']', '['}, {'{', '}'}, {'}', '{'} };

VisualObject::VisualObject() {
	x = y = renderPlane = 0;
	bOverrideInnateRules = 0;
	bHide = 0;
	setName("Visual");
}

VisualObject::~VisualObject() {
}

void VisualObject::destroy() {
	PrimitiveObject::destroy();
}

inline int VisualObject::render_order() { return x + y * getwidth(); }

void VisualObject::loop(float delta) {
	if (bDestroyed) return;
	PrimitiveObject::loop(delta);
}

std::vector <OutputData> VisualObject::readOutputDataFromFile(std::ifstream& file) {
	std::vector <OutputData> ret;
	bool done = false;
	char str[BUFFER_SIZE];
	int x, y = 0;
	while (!done) {
		file.getline(str, BUFFER_SIZE);
		if (str[0] == '#') { done = true; continue; }

		x = 0;
		int idx = 0;
		int len = strlen(str);
		while (idx < len) {
			while (str[idx] == ' ' && idx < len) ++idx, ++x;
			if (idx < len) {
				ret.push_back({ y, x, "" });
				while (str[idx] != ' ' && idx < len) {
					if (str[idx] == '5')
						ret.back().str += " ", ++idx, ++x;
					else
						ret.back().str += str[idx++], ++x;
				}
			}
		}   ++y;
	}   return ret;
}

void VisualObject::flipX(std::vector<OutputData>& data, int xAxis) {
	for (auto& it : data) {
		std::reverse(it.str.begin(), it.str.end());
		for (auto& ch : it.str) {
			if (flipRules.find(ch) != flipRules.end())
				ch = flipRules[ch];
		}
		it.x = xAxis - (it.x + it.len());
	}
}

std::vector<OutputData> LineDraw::draw(int x1, int y1, int x2, int y2, std::string alphabet) {
	std::vector <OutputData> ans;
	std::vector <std::pair <int, int>> points;

	if (y1 > y2) {
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	int v1 = abs(x1 - x2);
	int v2 = abs(y1 - y2);
	int diagonalDistance = v1 + 1;
	if (v1 < v2) diagonalDistance = v2 + 1;

	for (int i = 0; i < diagonalDistance; ++i) {
		float t = (diagonalDistance == 0 ? 0.0 : (float)i / diagonalDistance);
		int x3, y3;
		x3 = x1 + t * (x2 - x1);
		y3 = y1 + t * (y2 - y1);
		points.push_back({ x3, y3 });
	}

	for (int i = 0; i < points.size(); ++i) {
		char ch;
		if (i != 0) {
			if (points[i - 1].first == points[i].first)
				ch = alphabet[2];
			else if (points[i - 1].second == points[i].second)
				ch = alphabet[3];
			else if (points[i - 1].first < points[i].first)
				ch = alphabet[1];
			else
				ch = alphabet[0];
		}
		else {
			ch = '/';
		}
		ans.push_back({ -x2 + points[i].first, -y2 + points[i].second, std::string(1, ch)});
	}
	return ans;
}
