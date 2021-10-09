#ifndef ENCOUNTERLAYOUT_H
#define ENCOUNTERLAYOUT_H

#include "engine/Debugger.h"
#include "engine/VisualObject.h"

#include "Characters.h"

#include <vector>
#include <string>
#include <map>

#define LAYOUTS  GridLayoutManager::getInstance()
#define CHMATRIX std::vector <std::vector <char>>

class ScreenObject;
class GridTile;

#define PROFILE profiles[profilesIdx]

enum ANIM_MODE { FIRE_AND_FORGET = 0, PERSISTENT, LOOP };

struct grid_info {
	std::vector <GridTile*> tiles;
	std::vector <std::vector <int>> pivot_x;
	std::vector <std::vector <int>> pivot_y;
};

class GridShapeAnimation : public PrimitiveObject {
public:
	GridShapeAnimation(std::vector <std::pair <std::vector <OutputData>, float>> request, std::vector <GridTile*> targets, ANIM_MODE mode);

	std::vector <std::pair <std::vector <OutputData>, float>> request;
	std::vector <GridTile*> targets;
	std::vector <OutputData> resetShape;
	ANIM_MODE mode;
	int idx;

protected:
	void apply();
	virtual void loop(float delta) override;

	friend class GridLayoutManager;

	friend class GodClass;
	friend class Debugger;
};

class GridColorAnimation : public PrimitiveObject {
public:
	GridColorAnimation(std::vector <std::pair <Color, float>> request, std::vector <GridTile*> targets, ANIM_MODE mode);

	std::vector <std::pair <Color, float>> request;
	std::vector <GridTile*> targets;
	Color resetColor;
	ANIM_MODE mode;
	int idx;

protected:
	void apply();
	virtual void loop(float delta) override;

	friend class GridLayoutManager;

	friend class GodClass;
	friend class Debugger;
};

class GridLayoutManager {
public:
	~GridLayoutManager();
	inline static GridLayoutManager* getInstance() { return (instance ? instance : instance = new GridLayoutManager()); }

	void initializeGrid	(grid_info& grid, std::string name, int orig_x = 2, int orig_y = 50);
	void gridMark		(grid_info& grid, std::vector <std::pair <int, int>> tiles, int idx);
	void gridReset		(grid_info& grid);
	void gridClear		(grid_info& grid);

	void gridColor		(grid_info& grid, Color color);
	void gridColor		(grid_info& grid, std::vector <std::pair <int, int>> tiles, Color color);
	void gridDecolor	(grid_info& grid);
	void gridDecolor	(grid_info& grid, std::vector <std::pair <int, int>> tiles);

	void prioritize		(grid_info& grid, std::vector <std::pair <int, int>> tiles);
	void deprioritize	(grid_info& grid, std::vector <std::pair <int, int>> tiles);
	void deprioritize	(grid_info& grid);

	void gridAnimate	(grid_info& grid, std::vector <std::pair <int, float>>   req, std::vector <std::pair <int, int>> tiles, ANIM_MODE mode);
	void gridAnimate	(grid_info& grid, std::vector <std::pair <int, float>>   req, std::vector <GridTile*> tiles, ANIM_MODE mode);
	void gridAnimate	(grid_info& grid, std::vector <std::pair <Color, float>> req, std::vector <std::pair <int, int>> tiles, ANIM_MODE mode);
	void gridAnimate	(grid_info& grid, std::vector <std::pair <int, float>>   req, ANIM_MODE mode);
	void gridAnimate	(grid_info& grid, std::vector <std::pair <Color, float>> req, ANIM_MODE mode);

	std::vector <GridTile*> getTilesInRange(grid_info& grid, int range, int x, int y, std::vector <std::pair <int, int>>& v);
	std::vector <GridTile*> getTilesInRangeWithForbiddenRule(grid_info& grid, int range, int x, int y, std::vector <std::pair <int, int>>& v);
	std::vector <GridTile*> selectTiles(grid_info& grid, std::vector <std::pair <int, int>> tiles);
	
	int selectTile(grid_info& grid, std::pair <int, int> tile);

	std::map <std::pair <int, int>, int> calcDist					(grid_info& grid, int x, int y);
	std::map <std::pair <int, int>, std::pair <int, int>> AIHelper	(grid_info& grid, int x1, int y1, int x2, int y2);
	std::set <std::pair <int, int>> getForbiddenDestinationTiles	(grid_info& grid);

	void clearAnim(grid_info& grid);
	void clearAnim();
	void checkAnim();

	std::vector <std::pair <int, int>> getPath(grid_info& grid, const std::pair <int, int> &begin, const std::pair <int, int> &end, int &penalty);

	void paintTile(grid_info& grid, std::pair <int, int> tile, int shapeIndex = 0);

private:
	std::list <GridShapeAnimation*> shapeAnim;
	std::list <GridColorAnimation*> colorAnim;

	std::vector <std::vector <OutputData>> shapes;
	std::map <std::string, CHMATRIX> gridRawMap;

	int tileoff_x, tileoff_y;
	int tileadv_x, tileadv_y;

	std::string currentEncounter;

	std::vector <int> selectTiles(std::vector <std::pair <int, int>> tiles);

	static GridLayoutManager* instance;
	GridLayoutManager();

	friend class GodClass;
	friend class Debugger;
};

#endif