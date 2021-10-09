#ifndef AUDIOWORLDEMITTER_H
#define AUDIOWORLDEMITTER_H

#include "PrimitiveObject.h"

#include <fmod.hpp>

class AudioWorldEmitter : public PrimitiveObject
{
public:
	AudioWorldEmitter(const std::string &path, bool bLoop = false, float duration = 0.0f);

	struct rectangle { rectangle(int x1 = 0, int x2 = 0, int y1 = 0, int y2 = 0) : x1(x1), x2(x2), y1(y1), y2(y2) {}  int x1, x2, y1, y2; };

	inline void setSource(PrimitiveObject* src) { this->source = src; }
	inline void setPosition(int x, int y)       { this->x = x, this->y = y; }

	inline void setFocusArea(rectangle focus) { this->focus = focus; }
	inline void setAccomodationArea(rectangle accomodation) { this->accomodation = accomodation; }

	void adjustSoundChannel(FMOD::Channel *ch, int receptive_x, int receptive_y, float scale);

	void computeSoundChannelData(FMOD::Channel* ch, int receptive_x, int receptive_y, float &volume, float &pan, int sgn);

protected:
	virtual void loop(float delta) override;

	int			x, y;
	bool		bLoop;
	float		duration;
	rectangle   focus, accomodation;
	std::string path;

	inline int left	(const rectangle &area) { return x - area.x1; }
	inline int right(const rectangle &area) { return x + area.x2; }
	inline int up	(const rectangle &area) { return y - area.y1; }
	inline int down (const rectangle &area) { return y + area.y2; }

private:
	PrimitiveObject* source;

	friend class GodClass;
	friend class Debugger;
	friend class AudioManager;
};

#endif