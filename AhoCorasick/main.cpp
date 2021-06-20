#include "AhoCorasick.h"
#include <iterator>
#include <iostream>

template class AhoCorasickMachine<char>;

int main()
{
	AhoCorasickMachine<char>::FindResult<std::vector> result2;
	AhoCorasickMachine<char> machine({ "he", "she", "his", "hers" }, 'a', 'z');
	auto result = machine.find("ushers");

	machine.find("ushers", std::inserter(result2, result2.end()));

	return 0;
}