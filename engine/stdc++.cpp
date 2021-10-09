#include "stdc++.h"

#define USE_TESTLOG 0
#define	WRITE_LOG	0

#define LOG_NAME "log_save9.txt"

std::minstd_rand generator;
std::uniform_int_distribution<int> distribution(1, RAND_MAX_VALUE);

std::ofstream debuglog("engine/log.txt");
std::ifstream testlog(LOG_NAME);

void ResetRand() {
	generator.seed((unsigned int)(time(NULL)));
}

std::vector<std::string> split(std::string str, char delim) {
	std::vector <int> signs;
	std::vector <std::string> ans;
	signs.push_back(-1);
	if (str.empty()) return ans;
	if (str[str.size() - 1] != delim) str += delim;
	for (int i = 0; i < (int)str.size(); ++i) {
		if (str[i] == delim)
			signs.push_back(i);
	}
	for (int i = 1; i < (int)signs.size(); ++i) {
		int left = signs[i - 1] + 1;
		int right = signs[i] - 1;
		if (left <= right) ans.push_back(str.substr(left, right - left + 1));
	}	return ans;
}

std::vector<std::string> getNamesInsideFolder(std::string folder) {
	std::vector <std::string> ret;
	for (auto& entry : std::filesystem::directory_iterator(folder))
		ret.push_back(entry.path().string());
	return ret;
}

float randBetween(float min, float max) {
	return min + (max - min) * ((float)Rand() / RAND_MAX_VALUE);
}

std::string intToStr(int X) {
	std::stringstream ss;
	ss << X;
	return ss.str();
}

int strToInt(const std::string& str) {
	int num = 0, sgn = 1;
	int start = 0;
	if (str.size() > 0 && str[0] == '-')
		sgn = -1, start = 1;
	for (int i = start; i < str.size(); ++i)
		num = num * 10 + (str[i] - '0');
	return num*sgn;
}

int Rand() {
	if (USE_TESTLOG) 
		if (testlog.eof()) return rand();
		else { int x; testlog >> x; return x; }
	int v = distribution(generator);
	if (WRITE_LOG) debuglog << v << '\n';
	return v;
}