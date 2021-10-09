#ifndef DISJOINTSETUNION_H
#define DISJOINTSETUNION_H

#include <vector>

class DisjointSetUnion {
public:
	DisjointSetUnion(int size);

	int labelUnion(int label, int other);
	inline int labelRoot (int label);
	int labelSetMinimum(int label);

	void reset();

protected:
	std::vector <int> link, weight, min;

};

#endif
