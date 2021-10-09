#ifndef INMUTABLERENDERER_H
#define INMUTABLERENDERER_H

#include "ConsoleRenderer.h"
#include "DisjointSetUnion.h"

#include <set>
#include <vector>
#include <list>

struct OutputData;

class InmutableRenderer : public ConsoleRenderer
{
public:
	InmutableRenderer();
	~InmutableRenderer();

	virtual void addAnimator(Animator* anim);
	virtual void addGUI_Window(GUI_Window* win);
	virtual void addScreenObject(ScreenObject* obj);

	virtual void addBackground(std::string name, bool bLoop = false, float speed = 1);
	virtual void resetBackground();
	virtual void addScrollAmount(float amount);

protected:
	DisjointSetUnion DSU;

	virtual void render(float delta);

	virtual void manageRequests(float delta);
	virtual void managePrint(float delta);
	virtual void manageSegmentsOutput(float delta);

	struct renderRequest {
		renderRequest(std::pair <int, int> order);

		std::pair <int, int> order;
		virtual void update();
		virtual bool isDestroyed();
		virtual void run(InmutableRenderer* renderer) = 0;

		VisualObject *obj;

		bool operator < (const renderRequest& other) const {
			return order < other.order;
		}
	};

	struct animationRequest : public renderRequest {
		animationRequest(Animator* animation);

		Animator *animation;
		
		virtual void run(InmutableRenderer* renderer);
	};
	
	struct scbRequest : public renderRequest {
		scbRequest(ScreenObject *scb);
		
		ScreenObject *scb;

		virtual void run(InmutableRenderer* renderer);
	};
	
	struct winRequest : public renderRequest {
		winRequest(GUI_Window *win);
		
		GUI_Window *win;

		virtual void run(InmutableRenderer* renderer);
	};

	struct comparator {
		bool operator() (renderRequest* rq1, renderRequest* rq2) {
			return *rq1 < *rq2;
		}
	};

	std::list <renderRequest*> requests;
	std::list <renderRequest*> waitingRequests;

	virtual void addRequest(renderRequest* req);

	struct printSegment {
		int x1, x2;
		Color color;
		std::string str;
		int idx;
	};

	std::vector <printSegment> segments;

	virtual void addSegment(OutputData *data, Color color, VisualObject* obj, bool bPadding = true);

	struct renderData {
		std::string output;
		std::vector <Color> colors;
	}	prev, pres, reset;

private:
	virtual void calculateBackgroundOutput();

	COLOR computeColor(COLOR color);

	struct backgroundData {
		bool bLoop;
		int idx;
		std::vector <std::string> lines;
		float scrollSpeed;
		float scrollAmount;
	};
	std::vector <backgroundData> backgrounds;

	friend class Debugger;
};

#endif // INMUTABLERENDERER_H
