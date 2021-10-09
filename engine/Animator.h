#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "VisualObject.h"

#include <vector>

class AnimatorTable;
template <typename type>
class ObjectPositionXTransform;

struct Frame {
	int duration;
	std::vector <OutputData> data;
};

class Animator : public VisualObject
{
public:
	Animator(std::string asc_name, int renderPlane = 0, bool bPlay = true, bool fireAndForget = false);
	Animator(std::string asc_name, int x, int y, int renderPlane = 0, bool bPlay = true, bool fireAndForget = false);
	Animator(AnimatorTable *table, int tableIndex, int x = 0, int y = 0, int renderPlane = 0, bool bPlay = true, bool fireAndForget = false);
	Animator(AnimatorTable *table, std::string tableName, int x = 0, int y = 0, int renderPlane = 0, bool bPlay = true, bool fireAndForget = false);

	void updateAnimation(AnimatorTable* table, std::string str);
	
	virtual void destroy() override;

	inline void setSpeed(float value)		{ speed = value; }
	inline void setPosition(int x, int y)	{ this->x = x, this->y = y; }

	inline void play()		{ bPlaying = 1; }
	inline void pause()		{ bPlaying = 0; }
	inline void restart()	{ timer = 0; };
	inline void reset()		{ pause(), restart(), setSpeed(1), currentFrame = 0; }

	inline bool isPlaying() { return bPlaying; }

	static void setGlobalAnimSpeed(float value) { globalSpeed = value; }
	static void resetGlobalAnimSpeed()			{ globalSpeed = 1.0f; }

	void flipX(int xAxis);

	void overrideRenderPlane(int renderPlane) { this->renderPlane = renderPlane; }

	inline void setStartFrame(int value) { startFrame = value; }
	inline void setNextAnim(std::string name) { nextAnim = name; }
	inline void setAnimTable(AnimatorTable *table) { this->table = table; }
	
	static float getGlobalSpeed() { return globalSpeed; }

protected:
	virtual ~Animator();

	virtual void loop(float delta);

	static float globalSpeed;

	AnimatorTable *table;
	std::vector <Frame> frames;
	std::string nextAnim;
	int	  startFrame;
	int   currentFrame;
	bool  bPlaying;
	bool  fireAndForget;
	float speed;
	float timer;

	inline Frame getFrame() { return frames[currentFrame]; }

	inline bool isFrameOver() { return timer * FPS > getFrame().duration; }
	inline int advanceFrame() {
		timer -= (float)getFrame().duration * (FPS_UNIT);
		++currentFrame;
		if (currentFrame == (int)frames.size()) {
			if (fireAndForget) destroy(), currentFrame = (int) frames.size() - 1;
			else {
				if (table) updateAnimation(table, nextAnim), table = nullptr, startFrame = 0;
				currentFrame = startFrame;
			}
		}
		return currentFrame;
	}

private:
	static int objCount;
	
	void getDataFromFile(std::string asc_name);

	friend class InmutableRenderer;
	friend class ConsoleRenderer;

	friend class AnimatorTable;

	friend class GodClass;
	friend class Debugger;
};

#endif // ANIMATOR_H
