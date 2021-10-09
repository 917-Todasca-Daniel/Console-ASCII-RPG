#ifndef GARBAGETRUCK_H
#define GARBAGETRUCK_H

class GarbageTruck
{
public:
	GarbageTruck();
	virtual ~GarbageTruck();

	friend class Debugger;
};

#endif // GARBAGETRUCK_H
