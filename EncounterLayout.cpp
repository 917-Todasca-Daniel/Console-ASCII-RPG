#include "Encounter.h"

#include "engine/Timer.h"
#include "Behaviours.h"

#include <queue>

GridLayoutManager::GridLayoutManager() {
	std::ifstream file("resources/grid.layout");
	int nshapes; file >> nshapes; shapes.resize(nshapes); file.get();
	for (int i = 0; i < nshapes; ++i)
		shapes[i] = VisualObject::readOutputDataFromFile(file);
	file >> tileoff_x >> tileoff_y >> tileadv_x >> tileadv_y;
	std::string gridName;
	while (file >> gridName) {
		if (gridName == "#") return;
		auto& it = (gridRawMap[gridName] = CHMATRIX());
		char ch;
		while (file >> ch) {
			if (ch == '#') break;
			else {
				it.push_back(std::vector <char>());
				it.back().push_back(ch);
				while (file >> ch)
					if (ch == '#') break;
					else it.back().push_back(ch);
			}
		}
	}
}

GridLayoutManager::~GridLayoutManager() {
	clearAnim();
}

void GridLayoutManager::initializeGrid(grid_info& grid, std::string name, int orig_x, int orig_y) {
	currentEncounter = name;
	auto& it = gridRawMap[name];
	for (int i = 0, x, y, j; i < (int)it.size(); ++i) {
		grid.pivot_x.push_back(std::vector <int>());
		grid.pivot_y.push_back(std::vector <int>());
		for (j = 0; j < (int)it[i].size(); ++j) {
			x = orig_x + j * tileadv_x + (it.size() - i - 1) * tileoff_x;
			y = orig_y + (it.size() - i - 1) * tileadv_y;
			grid.pivot_x.back().push_back(x);
			grid.pivot_y.back().push_back(y);
			if (it[i][j] == '*') 
				continue;
			grid.tiles.push_back(new GridTile(x, y, 0));
			grid.tiles.back()->updateShape(shapes[0]);
		}
	}
}

void GridLayoutManager::gridMark(grid_info& grid, std::vector<std::pair<int, int>> tiles, int idx) {
	if (idx >= (int)shapes.size()) return;
	auto v = selectTiles(tiles);
	for (auto it : v) grid.tiles[it]->updateShape(shapes[idx]);
}
void GridLayoutManager::gridReset(grid_info& grid) {
	for (auto it : grid.tiles) it->updateShape(shapes[0]);
}

void GridLayoutManager::gridClear(grid_info& grid) {
	auto& map = gridRawMap[currentEncounter];
	int idx = 0;

	int first = -1;
	std::vector <std::vector <int>> lines;
	for (int i = 0; i < (int)map.size(); ++i) {
		bool bFirst = true;
		lines.push_back(std::vector <int>());
		for (int j = 0; j < (int)map[i].size(); ++j) {
			if (map[i][j] == '.') {
				if (first == -1) first = i;
				lines.back().push_back(idx);
				idx++;
			}
		}
	}

	float time_cycle = 0.2f;
	int line = 0;
	int c = 0;
	for (auto& it : lines) {
		for (auto& idx : it) {
			new FadeBehaviour <ScreenObject>((line - first) * time_cycle, time_cycle, grid.tiles[idx]);
			GridTile* grayCopy = new GridTile(grid.tiles[idx]->x, grid.tiles[idx]->y, --c);
			grayCopy->color = GRAY;
			grayCopy->updateShape(shapes[0]);
			grayCopy->hide();
			new Timer((line-first)*time_cycle, grayCopy, &GridTile::show);
			new FadeBehaviour <ScreenObject>(time_cycle/2 + (line - first) * time_cycle, time_cycle, grayCopy);
		}	++line;
	}
}

std::vector<int> GridLayoutManager::selectTiles(std::vector<std::pair<int, int>> tiles) {
	auto& map = gridRawMap[currentEncounter];
	std::sort(tiles.begin(), tiles.end(), [](std::pair <int, int>& p1, std::pair <int, int>& p2) {return std::pair <int, int>(p1.second, p1.first) < std::pair <int, int>(p2.second, p2.first); });
	std::vector <int> ret;
	int idx = 0, i = 0, j = 0;
	for (auto pair : tiles) {
		if (pair.second < map.size() && pair.first < map[pair.second].size() && map[pair.second][pair.first] != '*') {
			while (i != pair.first || j != pair.second) {
				if (j == map.size()) break;
				if (i == map[j].size()) i = 0, ++j;
				else {
					if (map[j][i] != '*') ++idx;
					++i;
				}
			}	ret.push_back(idx);
		}
	}	return ret;
}

std::vector<GridTile*> GridLayoutManager::getTilesInRange(grid_info& grid, int range, int x, int y, std::vector <std::pair <int, int>>& v) {
	auto map = gridRawMap[currentEncounter];
	std::queue <std::pair <int, int>> queue;
	std::vector <std::pair <int, int>> valids;
	queue.push({ x, y });
	int dx[] = { -1, 1, 0, 0 }, dy[] = { 0, 0, 1, -1 };
	std::set <std::pair <int, int>> set;
	set.insert({ x, y });
	while (!queue.empty()) {
		auto front = queue.front();
		queue.pop();
		if (abs(front.first - x) + abs(front.second - y) > range) continue;
		if (map[front.second][front.first] == '*') continue;
		valids.push_back(front);
		for (int i = 0; i < 4; ++i) {
			auto pair = std::pair <int, int>(front.first + dx[i], front.second + dy[i]);
			if (pair.second < map.size() && pair.first < map[pair.second].size()) {
				if (set.find(pair) == set.end())
					queue.push(pair), set.insert(pair);
			}
		}
	}	v = valids; return selectTiles(grid, valids);
}
std::vector<GridTile*> GridLayoutManager::getTilesInRangeWithForbiddenRule(grid_info& grid, int range, int x, int y, std::vector <std::pair <int, int>>& v) {
	auto map = gridRawMap[currentEncounter];
	std::queue <std::pair <int, int>> queue;
	std::vector <std::pair <int, int>> valids;
	queue.push({ x, y });
	int dx[] = { -1, 1, 0, 0 }, dy[] = { 0, 0, 1, -1 };
	std::set <std::pair <int, int>> set;
	set.insert({ x, y });

	std::set <std::pair <int, int>> forbidden;
	for (auto& it : Encounter::getInstance()->getProfiles()) {
		if (it->sentience == OBJECT)
			forbidden.insert(it->getTile());
	}

	while (!queue.empty()) {
		auto front = queue.front();
		queue.pop();
		if (abs(front.first - x) + abs(front.second - y) > range) continue;
		if (forbidden.find(front) != forbidden.end()) continue;
		if (map[front.second][front.first] == '*') continue;
		valids.push_back(front);
		for (int i = 0; i < 4; ++i) {
			auto pair = std::pair <int, int>(front.first + dx[i], front.second + dy[i]);
			if (pair.second < map.size() && pair.first < map[pair.second].size()) {
				if (set.find(pair) == set.end())
					queue.push(pair), set.insert(pair);
			}
		}
	}	v = valids; return selectTiles(grid, valids);
}
std::vector<GridTile*> GridLayoutManager::selectTiles(grid_info& grid, std::vector<std::pair<int, int>> tiles) {
	std::vector <GridTile*> ret;
	selectTiles(tiles);
	for (auto it : selectTiles(tiles)) ret.push_back(grid.tiles[it]);
	return ret;
}

int GridLayoutManager::selectTile(grid_info& grid, std::pair<int, int> pair) {
	auto& map = gridRawMap[currentEncounter];
	int idx = 0, i = 0, j = 0;
	if (pair.second < map.size() && pair.first < map[pair.second].size() && map[pair.second][pair.first] != '*') {
		while (i != pair.first || j != pair.second) {
			if (j == map.size()) break;
			if (i == map[j].size()) i = 0, ++j;
			else {
				if (map[j][i] != '*') ++idx;
				++i;
			}
		}
		if (i != pair.first || j != pair.second) idx = -1;
	}	else idx = -1;
	return idx;
}

std::set <std::pair <int, int>> GridLayoutManager::getForbiddenDestinationTiles(grid_info& grid) {
	std::set <std::pair <int, int>> set;
	for (auto& it : Encounter::getInstance()->getProfiles())
		set.insert({ it->x(), it->y() });
	return set;
}
std::map<std::pair<int, int>, std::pair <int, int>> GridLayoutManager::AIHelper(grid_info& grid, int x1, int y1, int x2, int y2) {
	std::map<std::pair<int, int>, std::pair <int, int>> ans;
	auto map1 = calcDist(grid, x1, y1);
	auto map2 = calcDist(grid, x2, y2);
	if (map1.size() != map2.size()) return ans;
	for (auto& it : map1) ans[it.first] = { it.second, map2[it.first] };
	return ans;
}
std::map<std::pair<int, int>, int> GridLayoutManager::calcDist(grid_info& grid, int x, int y) {
	std::map <std::pair <int, int>, int> ans;
	std::queue <std::pair <int, int>> queue;
	queue.push({ x, y });
	ans[{x, y}] = 0;


	int dx[] = { -1, 1, 0, 0 }, dy[] = { 0, 0, 1, -1 };
	auto map = gridRawMap[currentEncounter];

	std::set <std::pair <int, int>> forbidden;
	std::set <std::pair <int, int>> set;
	set.insert({ x, y });
	for (auto& it : Encounter::getInstance()->getProfiles()) {
		if (it->sentience == OBJECT)
			forbidden.insert(it->getTile());
	}

	while (!queue.empty()) {
		auto front = queue.front();
		queue.pop();

		if (forbidden.find(front) != forbidden.end()) continue;
		if (map[front.second][front.first] == '*') continue;
		for (int i = 0; i < 4; ++i) {
			auto pair = std::pair <int, int>(front.first + dx[i], front.second + dy[i]);
			if (pair.second >= 0 && pair.first >= 0 && pair.second < map.size() && pair.first < map[pair.second].size()) {
				if (set.find(pair) == set.end())
					queue.push(pair), set.insert(pair), ans[pair] = ans[front] + 1;
			}
		}
	}

	return ans;
}

void GridLayoutManager::gridColor(grid_info& grid, Color color) {
	for (auto it : grid.tiles) it->color = color;
}
void GridLayoutManager::gridColor(grid_info& grid, std::vector<std::pair<int, int>> tiles, Color color) {
	auto v = selectTiles(tiles);
	for (auto it : v) grid.tiles[it]->color = color;
}
void GridLayoutManager::gridDecolor(grid_info& grid) {
	gridColor(grid, Color());
}
void GridLayoutManager::gridDecolor(grid_info& grid, std::vector<std::pair<int, int>> tiles) {
	gridColor(grid, tiles, Color());
}

void GridLayoutManager::prioritize(grid_info& grid, std::vector<std::pair<int, int>> tiles) {
	auto v = selectTiles(tiles);
	for (auto it : v) grid.tiles[it]->renderPlane = 2;
}
void GridLayoutManager::deprioritize(grid_info& grid, std::vector<std::pair<int, int>> tiles) {
	auto v = selectTiles(tiles);
	for (auto it : v) grid.tiles[it]->renderPlane = 1;
}
void GridLayoutManager::deprioritize(grid_info& grid) {
	for (auto it : grid.tiles) it->renderPlane = 1;
}

void GridLayoutManager::gridAnimate(grid_info& grid, std::vector<std::pair<int, float>> req, std::vector<std::pair <int, int>> tiles, ANIM_MODE mode) {
	std::vector <std::pair <std::vector <OutputData>, float>> animData;
	for (auto it : req) animData.push_back({ shapes[it.first], it.second });
	shapeAnim.push_back(new GridShapeAnimation(animData, selectTiles(grid, tiles), mode));
	if (mode == FIRE_AND_FORGET) shapeAnim.back()->resetShape = shapes[0];
}
void GridLayoutManager::gridAnimate(grid_info& grid, std::vector<std::pair<int, float>> req, ANIM_MODE mode) {
	std::vector <std::pair <std::vector <OutputData>, float>> animData;
	for (auto it : req) animData.push_back({ shapes[it.first], it.second });
	shapeAnim.push_back(new GridShapeAnimation(animData, grid.tiles, mode));
	if (mode == FIRE_AND_FORGET) shapeAnim.back()->resetShape = shapes[0];
}
void GridLayoutManager::gridAnimate(grid_info& grid, std::vector<std::pair<int, float>> req, std::vector<GridTile*> tiles, ANIM_MODE mode) {
	std::vector <std::pair <std::vector <OutputData>, float>> animData;
	for (auto it : req) animData.push_back({ shapes[it.first], it.second });
	shapeAnim.push_back(new GridShapeAnimation(animData, tiles, mode));
	if (mode == FIRE_AND_FORGET) shapeAnim.back()->resetShape = shapes[0];
}

void GridLayoutManager::gridAnimate(grid_info& grid, std::vector <std::pair <Color, float>> req, std::vector <std::pair <int, int>> tiles, ANIM_MODE mode) {
	colorAnim.push_back(new GridColorAnimation(req, selectTiles(grid, tiles), mode));
}
void GridLayoutManager::gridAnimate(grid_info& grid, std::vector<std::pair<Color, float>> req, ANIM_MODE mode) {
	colorAnim.push_back(new GridColorAnimation(req, grid.tiles, mode));
}

void GridLayoutManager::clearAnim(grid_info& grid) {
	clearAnim();
	gridDecolor(grid);
	for (auto it : grid.tiles) it->updateShape(shapes[0]);
}
void GridLayoutManager::clearAnim() {
	for (auto it : shapeAnim) if (it) it->bDestroyed = true;
	for (auto it : colorAnim) if (it) it->bDestroyed = true;
}
void GridLayoutManager::checkAnim() {
	shapeAnim.remove(NULL);
	colorAnim.remove(NULL);
}

std::vector<std::pair<int, int>> GridLayoutManager::getPath(grid_info &grid, const std::pair<int, int>& begin, const std::pair<int, int>& end, int &penalty) {
	auto map = gridRawMap[currentEncounter];

	struct dijkMemory { std::pair <int, int> point; int cost; };

	std::priority_queue <std::pair <int, std::pair <int, int>>> queue;
	std::map <std::pair <int, int>, dijkMemory> ds;
	int x = begin.first;
	int y = begin.second;

	queue.push({ 0, std::pair <int, int>(x, y) });
	int dx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	int dy[] = { 0, 1, -1, 1, -1, 0, -1, 1 };
	ds[{x, y}] = { {-1, -1}, 0 };

	std::set <std::pair <int, int>> forbidden;
	for (auto& it : Encounter::getInstance()->getProfiles()) {
		if (it->sentience == OBJECT)
			forbidden.insert(it->getTile());
	}

	while (!queue.empty()) {
		auto front = queue.top();
		auto pt = front.second;
		queue.pop();
		auto& data = ds[pt];

		if (-front.first > data.cost) continue;
		if (map[pt.second][pt.first] == '*') continue;
		if (forbidden.find(pt) != forbidden.end()) continue;

		for (int i = 0; i < 8; ++i) {
			auto pair = std::pair <int, int>(pt.first + dx[i], pt.second + dy[i]);
			if (pair.second < map.size() && pair.first < map[pair.second].size()) {
				int val = data.cost + abs(dx[i]) + abs(dy[i]);
				if (ds.find(pair) == ds.end() || ds[pair].cost > val)
					queue.push({ -val, pair }), ds[pair] = { pt, val };
			}
		}
	}	

	std::pair <int, int> p = end;
	std::vector <std::pair <int, int>> ans;
	while (p.first != -1) {
		ans.push_back({ grid.pivot_x[p.second][p.first], grid.pivot_y[p.second][p.first] });
		p = ds[p].point;
	}	std::reverse(ans.begin(), ans.end());

	penalty = ds[end].cost;

	return ans;
}

void GridLayoutManager::paintTile(grid_info &grid, std::pair<int, int> tile, int shapeIndex) {
	int idx = selectTile(grid, tile);
	if (idx == -1) return;
	grid.tiles[idx]->updateShape(shapes[shapeIndex]);
}

GridShapeAnimation::GridShapeAnimation(std::vector<std::pair<std::vector <OutputData>, float>> request, std::vector <GridTile*> targets, ANIM_MODE mode) :
	request(request), mode(mode), idx(0), targets(targets) {
	setName("gr1Anim");
	apply();
}
void GridShapeAnimation::loop(float delta) {
	if (idx >= (int)request.size()) { destroy();  return; }
	PrimitiveObject::loop(delta);
	if (bDestroyed) return;
	while (idx < (int)request.size() && lifespan > request[idx].second) {
		lifespan -= request[idx++].second;
		if (idx == request.size()) {
			if (mode == LOOP) idx = 0, apply();
			else if (mode == FIRE_AND_FORGET) {
				if (resetShape.size() != 0) for (auto it : targets)
					it->updateShape(resetShape);
				destroy();
			}
		}
		else apply();
	}
}
void GridShapeAnimation::apply() { for (auto it : targets) it->updateShape(request[idx].first); }

GridColorAnimation::GridColorAnimation(std::vector<std::pair<Color, float>> request, std::vector <GridTile*> targets, ANIM_MODE mode) :
	request(request), mode(mode), idx(0), targets(targets), resetColor(Color()) {
	apply();
	setName("gr2Anim");
}
void GridColorAnimation::loop(float delta) {
	if (idx >= (int)request.size()) { destroy();  return; }
	PrimitiveObject::loop(delta);
	if (bDestroyed) return;
	while (idx < (int)request.size() && lifespan > request[idx].second) {
		lifespan -= request[idx++].second;
		if (idx == request.size()) {
			if (mode == LOOP) idx = 0, apply();
			else if (mode == FIRE_AND_FORGET) {
				for (auto it : targets)
					it->color = resetColor;
				destroy();
			}
		}
		else apply();
	}
}
void GridColorAnimation::apply() { for (auto it : targets) it->color = request[idx].first; }