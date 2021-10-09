#ifndef BEHAVIOURS_H
#define BEHAVIOURS_H

#include "engine/PrimitiveObject.h"
#include "engine/Animator.h"

#define _WIN32_WINNT 0x0500

class PrimitiveBehaviour : public PrimitiveObject {
public:
	PrimitiveBehaviour(PrimitiveObject *obj) { this->target = obj; };

protected:
	PrimitiveObject *target;

	virtual void loop(float delta) override {
		if (bDestroyed) return;
		if (target == nullptr || target->wasDestroyed()) destroy();
		PrimitiveObject::loop(delta);
	}

	void destroyTarget() { if (target) target->destroy(); }

	friend class GodClass;
	friend class Debugger;
};

template <typename type>
class ObjectPositionXTransform : public PrimitiveBehaviour {
public:
	ObjectPositionXTransform(type* obj, float speed, float accel, int target, bool bTargetDestruction = false);

protected:
	virtual void loop(float delta) override;

	int sgn, pos, target;
	bool bTargetDestruction;
	float speed, accel;
	type *obj;
};

template <typename type>
class CombatGridLineTraceBehaviour : public PrimitiveBehaviour {
public:
	CombatGridLineTraceBehaviour(std::vector <std::pair <int, int>> points, type *obj, int speed);

protected:
	virtual void loop(float delta) override;
	
	int   speed;
	int   idx;
	float x, y;
	type  *obj;
	std::vector <std::pair <int, int>> points;

	friend class GodClass;
	friend class Debugger;
};

template<typename type>
CombatGridLineTraceBehaviour<type>::CombatGridLineTraceBehaviour(std::vector<std::pair<int, int>> points, type *obj, int speed) : speed(speed), points(points), PrimitiveBehaviour(obj) {
	this->obj = obj; 
	idx = 0;
	x = (float) obj->x;
	y = (float) obj->y;
	setName("GridBehaviour");
}

template<typename type>
void CombatGridLineTraceBehaviour<type>::loop(float delta) {
	PrimitiveBehaviour::loop(delta);

	if (bDestroyed) return;
	if (idx >= (int) points.size() || idx < 0) {
		destroy();
		sendNotifier("movement behaviour finished");
		return;
	}	float quantity = delta * speed;

	float modif = 1.2f;
	if (points[idx].second - y != 0) modif = 0.7f;

	if      (points[idx].first  - x < 0) x = max((float)points[idx].first,  x - modif*quantity);
	else if (points[idx].first  - x > 0) x = min((float)points[idx].first,  x + modif*quantity);
	if      (points[idx].second - y < 0) y = max((float)points[idx].second, y - 0.4f*modif*quantity);
	else if (points[idx].second - y > 0) y = min((float)points[idx].second, y + 0.4f*modif*quantity);

	if (std::pair <int, int>((int)x, (int)y) == points[idx]) ++idx;

	obj->setPosition((int)x, (int)y);
}

template <typename type>
class FlashBehaviour : public PrimitiveBehaviour {
public:
	FlashBehaviour(int ticks, float durationIn, float durationOut, type *obj, bool endState = true);

protected:
	virtual void loop(float delta) override;

	int	  ticks;
	int   currentTicks;
	float currentTime;
	float durationIn, durationOut;
	bool  endState;
	type  *obj;

	friend class GodClass;
	friend class Debugger;
};

template <typename type>
class LeftToRightScan : public PrimitiveBehaviour {
public:
	LeftToRightScan(float speed, float delay, type *obj);
	LeftToRightScan(int min_x, float speed, float delay, type *obj);

protected:
	virtual void loop(float delta);

	type *target;
	float speed;
	float amount;
	float delay;
	int min_x, max_x;

	std::vector <OutputData> dataCopy;

	friend class GodClass;
	friend class Debugger;
};

template <typename type>
class ScrollAppearance : public PrimitiveBehaviour {
public:
	ScrollAppearance(float speed, type *obj);

protected:
	virtual void loop(float delta);

	type *target;
	float speed;
	int min_x, max_x;

	std::vector <OutputData> dataCopy;

	friend class GodClass;
	friend class Debugger;
};

template<typename type>
inline FlashBehaviour<type>::FlashBehaviour(int ticks, float durationIn, float durationOut, type *obj, bool endState) : endState(endState), durationIn(durationIn), durationOut(durationOut), ticks(ticks), obj(obj), PrimitiveBehaviour(obj) {
	currentTicks = 0;
	currentTime = 0;
	setName("FlashBehaviour");
}

template<typename type>
inline void FlashBehaviour<type>::loop(float delta) {
	PrimitiveBehaviour::loop(delta);

	if (bDestroyed) return;

	currentTime += delta;
	while (currentTime > durationIn + durationOut) ++currentTicks, currentTime -= durationIn + durationOut;
	if (currentTicks >= ticks) {
		destroy();
		if (endState) obj->show();
		else obj->hide();
	}
	else {
		if (currentTime < durationOut) obj->hide();
		else obj->show();
	}
}

template <typename type>
class FadeBehaviour : public PrimitiveBehaviour {
public:
	FadeBehaviour(float delay, float time, type *obj, int sgn = 1);

protected:
	virtual void loop(float delta) override;

	float delay, time;
	float currentTime;
	type  *obj;

	int min, max;
	int sgn;

	friend class GodClass;
	friend class Debugger;
};

template<typename type>
inline FadeBehaviour<type>::FadeBehaviour(float delay, float time, type *obj, int sgn) : delay(delay), time(time), obj(obj), sgn(sgn), PrimitiveBehaviour(obj) {
	currentTime = 0;
	min = (int) 2e9;
	max = (int) -2e9;
	if (!obj->data.empty()) {
		min = obj->data[0].y;
		max = obj->data[0].y;
		for (auto& it : obj->data) {
			if (it.y < min) min = it.y;
			if (it.y > max) max = it.y;
		}
	}
	setName("FadeBehaviour");
}

template<typename type>
inline void FadeBehaviour<type>::loop(float delta) {
	PrimitiveBehaviour::loop(delta);

	if (bDestroyed) {
		destroyTarget();
		return;
	}

	currentTime += delta;
	if (currentTime > delay) {
		float len = (float) max - min + 1;
		float tick = time / len;
		int segments = (int)((currentTime - delay) / tick);
		int value = 0;
		if (sgn == -1) {
			value = max - segments;
			obj->cut([&value](OutputData& a)->bool {
				return a.y >= value;
			});
		}
		else {
			value = min + segments;
			obj->cut([&value](OutputData& a)->bool {
				return a.y <= value;
			});
		}

		if (currentTime > delay + time) destroyTarget();
	}
}

#endif // BEHAVIOURS_H	

template<typename type>
inline ObjectPositionXTransform<type>::ObjectPositionXTransform(type* obj, float speed, float accel, int target, bool bTargetDestruction) : speed(speed), target(target), pos(obj->getPosition().first), 
	accel(accel), bTargetDestruction(bTargetDestruction), PrimitiveBehaviour(obj) {
	this->obj = obj;
	setName("transformX");
	sgn = 1;
	if (target < pos) sgn = -1;
}

template<typename type>
inline void ObjectPositionXTransform<type>::loop(float delta) {
	PrimitiveBehaviour::loop(delta);
	if (bDestroyed) return;
	
	float accel = this->accel * Animator::getGlobalSpeed();
	lifespan += accel * delta - delta;
	float value = pow(speed, lifespan);
	int candidate = pos + (int)value*sgn;
	obj->setPosition(candidate, obj->getPosition().second);
	if (candidate * sgn > target * sgn) {
		destroy();
		if (bTargetDestruction) destroyTarget();
	}
}

template<typename type>
inline ScrollAppearance<type>::ScrollAppearance(float speed, type* obj) : speed(speed), target(obj),  PrimitiveBehaviour(obj) {
	if (target->data.empty()) destroy();
	else min_x = target->data[0].x, max_x = target->data[0].x + (int)target->data[0].str.size();
	for (auto it : target->data) {
		min_x = min(min_x, it.x);
		max_x = max(max_x, it.x + (int)it.str.size() - 1);
	}	dataCopy = obj->data;
}


template<typename type>
inline void ScrollAppearance<type>::loop(float delta) {
	if (dataCopy.empty()) destroy();
	PrimitiveBehaviour::loop(delta);

	if (bDestroyed) return;

	float amount = lifespan * speed * speed;
	int value = (int)amount;

	if (value > (max_x - min_x)/2)
		value = (max_x - min_x)/2;

	std::vector <OutputData> result;
	for (auto it : dataCopy) {
		std::string str = "";
		for (int i=0; i<(int)it.str.size(); ++i) {
			int x = it.x + i;
			if (x - min_x <= value || max_x - x <= value)
				str += it.str[i];
		}
		if (str != "") {
			int x = it.x;
			x = min_x + (max_x - min_x) / 2 - value;
			result.push_back({ it.y, x, str });
		}
	}

	target->data = result;
	if (value >= (max_x-min_x)/2) destroy();
}

template <typename type>
class ShakeBehaviour : public PrimitiveBehaviour {
public:
	ShakeBehaviour(float time, float cycle, std::vector <type*> objs);

	virtual void destroy();

protected:
	virtual void loop(float delta) override;

	void doCycle();

	std::pair <int, int> offset;
	float time, cycle;
	float counter;
	std::vector <type*> objs;
	std::vector <std::pair <int, int>> origCopy;

	friend class GodClass;
	friend class Debugger;
};

template<typename type>
inline ShakeBehaviour<type>::ShakeBehaviour(float time, float cycle, std::vector<type*> objs) : time(time), cycle(cycle), counter(0), offset({ 0, 0 }), objs(objs), PrimitiveBehaviour(objs[0]) {
	for (auto it : objs) {
		origCopy.push_back(it->getPosition());
	}
}

template<typename type>
inline void ShakeBehaviour<type>::destroy() {
	PrimitiveBehaviour::destroy();
	for (int i = 0; i < objs.size(); ++i) {
		objs[i]->setPosition(origCopy[i]);
	}
}

template<typename type>
inline void ShakeBehaviour<type>::loop(float delta) {
	PrimitiveBehaviour::loop(delta);

	if (wasDestroyed()) return;

	if (lifespan > time && !wasDestroyed()) {
		destroy();
		return;
	}

	counter += delta;
	while (counter >= cycle) {
		doCycle();
		counter -= cycle;
	}
}

template<typename type>
inline void ShakeBehaviour<type>::doCycle() {
	int v1 = Rand() % 2;
	int v2 = Rand() % 2;
	
	if (offset.first == 2) -- offset.first;
	else if (offset.first == -2) ++ offset.first;
	else offset.first += (v1 * 2 - 1);
	
	if (offset.second == 2) -- offset.second;
	else if (offset.second == -2) ++ offset.second;
	else offset.second += (v2 * 2 - 1);

	for (int i = 0; i < objs.size(); ++i) {
		int x = origCopy[i].first + offset.first;
		int y = origCopy[i].second + offset.second;
		objs[i]->setPosition({ x, y });
	}
}

template<typename type>
inline LeftToRightScan<type>::LeftToRightScan(float speed, float delay, type* obj) : speed(speed), delay(delay), target(obj), PrimitiveBehaviour(obj) {
	if (target->data.empty()) destroy();
	min_x = 2e9;
	max_x = -2e9;
	for (auto it : target->data) 
		min_x = min(min_x, it.x), max_x = max(max_x, it.x);
	dataCopy = obj->data;
	obj->data.clear();
}

template<typename type>
inline LeftToRightScan<type>::LeftToRightScan(int min_x, float speed, float delay, type* obj) : speed(speed), delay(delay), target(obj), PrimitiveBehaviour(obj) {
	if (target->data.empty()) destroy();
	this->min_x = min_x;
	max_x = -2e9;
	for (auto it : target->data)
		max_x = max(max_x, it.x);
	dataCopy = obj->data;
	obj->data.clear();
	this->min_x -= target->getPosition().first;
}

template<typename type>
inline void LeftToRightScan<type>::loop(float delta) {
	if (dataCopy.empty()) destroy();
	PrimitiveBehaviour::loop(delta);

	if (bDestroyed) return;

	if (lifespan < delay) return;

	amount += delta * speed;
	if (amount + min_x >= max_x) {
		target->data = dataCopy;
		destroy();
		return;
	}
	else {
		int cursor = min_x + amount;
		for (auto dataPiece : dataCopy) {
			if (dataPiece.x > cursor) continue;
			OutputData pieceCopy = dataPiece;
			if (dataPiece.x + dataPiece.len() > cursor) {
				int len = cursor - dataPiece.x;
				pieceCopy.str = dataPiece.str.substr(0, len);
			}
			target->data.push_back(pieceCopy);
		}
	}
}
