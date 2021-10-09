#ifndef SCREENOBJECTTABLE_H
#define SCREENOBJECTTABLE_H

#include "GarbageTruck.h"

#include <string>
#include <vector>
#include <map>

class ScreenObject;
struct OutputData;

class ScreenObjectTable : public GarbageTruck
{
public:
	ScreenObjectTable(std::string tscb_name);
	virtual ~ScreenObjectTable();

	void updateShape(ScreenObject *obj, int tableIndex);
	void updateShape(ScreenObject *obj, std::string tableName);

protected:
	std::map <std::string, int> link;
	std::vector <std::vector <OutputData>> shapes;

private:

	friend class Debugger;
};

#endif // SCREENOBJECTTABLE_H
