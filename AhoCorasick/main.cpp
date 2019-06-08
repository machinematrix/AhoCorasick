
#include "AhoCorasick.h"
#include <iterator>

template class AhoCorasickMachine<char>;

int main()
{
	AhoCorasickMachine<char> machine({ "he", "she", "his", "hers" }, 'a', 'z');
	std::vector<std::pair<size_t, decltype(machine)::StringType>> result2;

	machine.find("ushers", std::back_inserter(result2));

	return 0;
}