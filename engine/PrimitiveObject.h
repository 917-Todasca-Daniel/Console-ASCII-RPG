#ifndef PRIMITIVEOBJECT_H
#define PRIMITIVEOBJECT_H

#include "GodClass.h"

#include <map>
#include <set>

class PrimitiveObject
{
public:
	PrimitiveObject();

	inline virtual void destroy() { bDestroyed = 1; }
	bool wasDestroyed() { return bDestroyed; }

	void setName(std::string root) { eraseFromNameMapTable(); this->rootName = root; this->stringIdentifier = addToNameMapTable(); }
	inline std::string getIdentifierName() { return this->stringIdentifier; }

	PrimitiveObject *parent;

protected:
	virtual ~PrimitiveObject();

	virtual void loop(float delta);

	float lifespan;
	bool  bDestroyed;
	bool  bLoopPostcedence;

	std::string rootName, stringIdentifier;
	static std::map <std::string, int> nameMap;

	void eraseFromNameMapTable();
	std::string addToNameMapTable();

	static std::set <std::string> notifiers;
	void sendNotifier(std::string notifier);
	bool checkNotifier(std::string notifier);
	void removeNotifier(std::string notifier);

private:
	static int obj_count;

	friend class PrimitiveBehaviour;
	template <typename type> friend class DisplayBox;

	friend class GodClass;

	friend class Debugger;
};

#endif // PRIMITIVEOBJECT_H
