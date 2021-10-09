#ifndef TIMER_H
#define TIMER_H

#include "PrimitiveObject.h"
#include "Delegates.h"

class Timer : public PrimitiveObject
{
public:
	Timer(float duration, void (*event)());
	template <typename type>
	Timer(float duration, type *obj, void (type::* event)());

	void resetTime();

protected:
	virtual ~Timer();

	virtual void loop(float delta) override;
	float duration;

	bool bDelegated;
	union {
		void (*callback)();
		PrimitiveClassMethodDelegate <void>* delegate;
	};

private:

	friend class Debugger;
};

template <typename type>
Timer::Timer(float duration, type *obj, void (type::* event)()) : duration(duration) {
	delegate = new ClassMethodDelegate <void, type>(obj, event);
	bDelegated = 1; setName("Timer");
}


#endif // TIMER_H
