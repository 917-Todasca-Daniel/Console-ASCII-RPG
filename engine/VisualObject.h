#ifndef VISUALOBJECT_H
#define VISUALOBJECT_H

#include "PrimitiveObject.h"

#include <map>
#include <fstream>

struct OutputData {
	int y, x;
	std::string str;
	inline int len() const { return str.size(); }
	inline int x2()  const { return x + len() - 1; }
};

class VisualObject : public PrimitiveObject
{
public:
	VisualObject();

	virtual void destroy() override;

	virtual inline int priority() { return renderPlane; };
	virtual inline int render_order();

	inline virtual void setPosition(int x, int y) { this->x = x, this->y = y; }
	inline virtual void setPosition(std::pair <int, int> pair) { this->x = pair.first, this->y = pair.second; }
	inline std::pair <int, int> getPosition() { return std::pair <int, int>(x, y); }

	inline virtual void hide() { bHide = 1; }
	inline virtual void show() { bHide = 0; }

	inline virtual bool isHidden() { return bHide; }

	template <typename Functor>
	static void cut(Functor condition, std::vector <OutputData>& data) {
		std::vector <OutputData> formatted;
		for (auto& it : data) {
			if (!condition(it)) formatted.push_back(it);
		}	data = formatted;
	}

	Color color;

	static std::vector <OutputData> readOutputDataFromFile(std::ifstream& file);
	static void flipX(std::vector <OutputData>& data, int xAxis);

protected:
	virtual ~VisualObject();

	virtual void loop(float delta) override;

	int x, y;
	int renderPlane;
	bool bOverrideInnateRules;
	bool bHide;

private:
	static std::map <char, char> flipRules;

	friend class InmutableRenderer;
	friend class ConsoleRenderer;

	friend class ScreenObjectTable;
	friend class AnimatorTable;

	template <typename type> friend class DisplayBox;

	friend class Debugger;
};

class LineDraw {
public:
	static std::vector <OutputData> draw(int x1, int y1, int x2, int y2, std::string alphabet = "--..");
};

#endif // VISUALOBJECT_H
