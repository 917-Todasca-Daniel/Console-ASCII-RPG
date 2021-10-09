#ifndef STDC_H
#define STDC_H

#include <iostream>
#include <string>
#include <time.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <random>
#include <filesystem>
#include <queue>

#define RAND_MAX_VALUE 54325

int  Rand();
void ResetRand();

std::vector <std::string> split(std::string str, char delim);

std::vector <std::string> getNamesInsideFolder(std::string folder);

float randBetween(float min, float max);

template <typename type>
type randomArrElement(std::vector <type> arr);

template <typename type>
type randomArrElementWeighted(std::vector <std::pair <float, type>> arr);

template<typename type>
inline type randomArrElement(std::vector<type> arr) {
	if (arr.empty()) return type();
	int v = Rand() % (arr.size());
	return arr[v];
}

template <typename type>
inline type randomArrElementWeighted(std::vector <std::pair <float, type>> arr) {
	if (arr.empty()) return type();
	float sum = 0;
	for (auto& it : arr) sum += it.first;
	int v = (int) fmod(Rand(), sum);
	for (auto& it : arr) {
		v -= (int) it.first;
		if (v <= 0) return it.second;
	}	return type();
}

template<typename type>
inline void random_shuffle(type a, type b) {
	std::random_device rd;
	std::mt19937 g(rd());
	shuffle(a, b, g);
}

std::string intToStr(int X);
int strToInt(const std::string &str);

#endif // STDC_H