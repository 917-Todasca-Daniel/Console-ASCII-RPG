#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

class Parser {
public:
	Parser(std::string text);
	bool contains(std::string cmd);
	std::string getOriginalText() { return originalText; }
	std::string getErrorMessage() {
		if (originalText.empty()) return "Command was empty. What are you, a mute?";
		else return "Invalid command: '" + originalText + "'.";
	}

private:
	std::string originalText;
	std::vector <std::string> words;
};

class Match {
public:
	static bool match(std::string blueprint, std::string answer);
};

#endif	// PARSER_H

