#include "DisjointSetUnion.h"

DisjointSetUnion::DisjointSetUnion(int size) {
	link.resize(size);
	weight.resize(size);
	min.resize(size);
	for (int i = 0; i < size; ++i)
		link[i] = min[i] = i;
}

int DisjointSetUnion::labelUnion(int label, int other) {
	int u = labelRoot(label);
	int v = labelRoot(other);
	if (u == v) return min[u];

	int val = min[u];
	if (val > min[v]) val = min[v];
	int p;

	if (weight[u] < weight[v]) link[u] = p = v;
	else                       link[v] = p = u;
	if (weight[u] == weight[v]) ++ weight[u];
	
	min[p] = val;
	return val;
}

int DisjointSetUnion::labelRoot(int label) {
	if (link[label] == label) return label;
	return link[label] = labelRoot(link[label]);
}

int DisjointSetUnion::labelSetMinimum(int label) {
	return min[labelRoot(label)];
}

void DisjointSetUnion::reset() {
	for (int i = 0; i < (int)link.size(); ++i) {
		link[i] = i;
		weight[i] = 0;
		min[i] = i;
	}
}
