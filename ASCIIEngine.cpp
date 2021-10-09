#include "engine/Debugger.h"

#include "engine/GodClass.h"

#include "engine/stdc++.h"

Debugger* debug;
void mainInitialize() {
	ResetRand();
	std::cout << "Initializing God class...\n";
	GOD;	// initializes GodClass
	std::cout << "Initialized God!\n";
	std::cin.tie(NULL);
	std::cout.tie(NULL);
	srand((unsigned int)time(NULL));

	std::cout << "Initializing debugger...\n";
	debug = new Debugger();
	std::cout << "Initialized debugger!\n";
	if (DEBUG_MODE) debug->initialize(), GOD->setDebugger(debug);
}

int main()
{
	std::cout << "Main initialize...\n";
	mainInitialize();
	std::cout << "Main initialize done!\n";

	std::clock_t present, time;
	float seconds;

	time = clock();

	float total = 0;
	std::cout << "Beginning loop...\n";
	while (true) {
		present = clock(); seconds = ((present - time) / (float)CLOCKS_PER_SEC);
		if (seconds < 0.01f) continue;
		total += seconds;
		GOD->loop(seconds);
		if (debug) debug->loop(seconds);
		time = present;
		if (total > 200000000.0f) {
			delete GOD;
			break;
		}
	}
	std::cout << "Ending loop...\n"; 
	
	if (debug) delete debug;
	std::cout << "Deleted debug...\n";
	
	return 0;
}
