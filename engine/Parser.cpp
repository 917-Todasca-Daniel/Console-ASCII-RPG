#include "Parser.h"

#include "stdc++.h"

#include "Debugger.h"

Parser::Parser(std::string text) {
	auto arr = split(text, ' ');
	for (int len = 0, i = 0; i < (int)arr.size() && words.size() < 5; ++i) {
		if (arr[i].size() > 1) words.push_back(arr[i]);
	}	originalText = text;
}

bool Parser::contains(std::string cmd) {
	for (auto& it : words) if (it == cmd) return true;;
	return false;
}

bool Match::match(std::string blueprint, std::string answer) {
	Parser parser(answer);
	auto synonyms = split(blueprint, '|');
	for (auto variant : synonyms) {
		auto arr = split(variant, ' ');
		bool bMatch = true;	
		for (auto v : arr) {
			if (!parser.contains(v))
				bMatch = false;
		}

		if (bMatch) return true;
	}	return false;
}