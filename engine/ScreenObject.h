#ifndef SCREENOBJECT_H
#define SCREENOBJECT_H

#include "VisualObject.h"
#include "Colors.h"

#include <fstream>

class ScreenObjectTable;

class ScreenObject : public VisualObject
{
public:
	ScreenObject(int x = 0, int y = 0, int priority = 0, std::string scb = "");
	ScreenObject(int x, int y, int priority, ScreenObjectTable* table, std::string tableName);
	ScreenObject(int x, int y, int priority, ScreenObjectTable* table, int tableIdx);

	virtual void destroy() override;

	void updateShape(ScreenObjectTable *table, std::string tableName);
	void updateShape(ScreenObjectTable *table, int tableIndex);
	void updateShape(ScreenObject *obj, std::vector <OutputData> shape, std::vector <int> lens, bool bFormat = false);
	void updateShape(std::string str);
	void updateShape(std::string fill, std::vector <OutputData> shape, std::vector <int> lens, bool bFormat = false);
	void updateShape(std::ifstream& file);

	inline void updateShape(std::vector <OutputData> data) { this->data = data; }
	std::vector <OutputData> data;

	inline int estimateWidth() { 
		int sum = 0;
		for (auto it : data) sum += it.str.size() + 1;
		return sum; 
	}

	template <typename Functor>
	inline void cut(Functor condition) {
		VisualObject::cut(condition, data);
	}

	inline void flipX(int xAxis) {
		VisualObject::flipX(data, xAxis);
	}

protected:
	virtual ~ScreenObject();

	virtual void loop(float delta) override;

private:
	static int objCount;

	friend class InmutableRenderer;
	friend class ConsoleRenderer;

	friend class ScreenObjectTable;

	friend class GodClass;

	friend class Debugger;
};

class GridTile : public ScreenObject {
public:
	GridTile(int x = 0, int y = 0, int priority = 0, std::string scb = "") : ScreenObject(x, y, priority, scb)  { setName("tile"); };
	GridTile(int x, int y, int priority, ScreenObjectTable *table, std::string tableName) : ScreenObject(x, y, priority, table, tableName) { setName("tile"); };
	GridTile(int x, int y, int priority, ScreenObjectTable *table, int tableIdx) : ScreenObject(x, y, priority, table, tableIdx) { setName("tile"); };

	virtual int render_order() override;

	inline virtual void show() override { VisualObject::show(); }

	friend class GridLayoutManager;

	friend class Debugger;
};

class TextFloatingPopup : public ScreenObject {
public:
	TextFloatingPopup(std::string text, float lifespan, float speed, int x, int y, int priority = 100, COLOR color = DRED) : orx(x), ory(y), maxtime(lifespan), speed(speed), amount(0), ScreenObject(x, y, priority, "") { 
		updateShape(text);
		show();
		this->color = color;
		setName("floating");
	};

protected:
	virtual void loop(float delta) override;

private:
	float maxtime, speed, amount;
	int orx, ory;

	friend class Debugger;
};

#endif // SCREENOBJECT_H
