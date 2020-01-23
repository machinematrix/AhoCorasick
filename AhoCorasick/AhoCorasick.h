#ifndef __AHOCORASICK__
#define __AHOCORASICK__
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <set>

template <typename ChTy>
class AhoCorasickMachine
{
public:
	typedef std::basic_string<ChTy, std::char_traits<ChTy>, std::allocator<ChTy>> StringType;
private:
	const ChTy firstToken, lastToken;
	std::vector<std::vector<size_t>> gotoFunc; //filas: estados | columnas: letras en el rango [firstToken;lastToken]
	std::map<size_t, std::set<StringType>> outputFunc;
	std::vector<size_t> failureFunc;
	std::vector<char> queryFunc; //tells us whether a state is an output state, without having to check outputFunc, which is slower.
public:
	AhoCorasickMachine(const ChTy firstToken = 'a', const ChTy lastToken = 'z');
	template <typename InputIterator> AhoCorasickMachine(InputIterator first, InputIterator last, const ChTy firstToken = 'a', const ChTy lastToken = 'z');
	AhoCorasickMachine(std::initializer_list<StringType> il, const ChTy firstToken = 'a', const ChTy lastToken = 'z');
	template <typename OutputIterator> void find(const StringType &haystack, OutputIterator output) const;
	void addWord(const StringType &word);
};
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
template <typename ChTy>
AhoCorasickMachine<ChTy>::AhoCorasickMachine(const ChTy firstToken, const ChTy lastToken)
	:firstToken(firstToken),
	lastToken(lastToken),
	gotoFunc(1, std::vector<size_t>(lastToken - firstToken + 1, 0)),
	failureFunc(1, 0), //failure function can be accessed safely with a default initialized object
	queryFunc(1, false) //we don't also initialize outputFunc because this state is not an output state, and outputFunc contains only output states
{}

template <typename ChTy>
template <typename InputIterator>
AhoCorasickMachine<ChTy>::AhoCorasickMachine(InputIterator first, InputIterator last, const ChTy firstToken, const ChTy lastToken)
	:AhoCorasickMachine(firstToken, lastToken)
{
	if (lastToken <= firstToken) throw std::logic_error("Invalid alphabet");

	while (first != last) //armar arbol de prefijos
		addWord(*first++);
}

template <typename ChTy>
AhoCorasickMachine<ChTy>::AhoCorasickMachine(std::initializer_list<StringType> il, const ChTy firstToken, const ChTy lastToken)
	:AhoCorasickMachine(il.begin(), il.end(), firstToken, lastToken)
{}

template <typename ChTy>
template <typename OutputIterator>
void AhoCorasickMachine<ChTy>::find(const typename StringType &haystack, OutputIterator output) const
{
	size_t state = 0;

	for (typename StringType::size_type i = 0; i < haystack.size(); ++i)
	{
		while (gotoFunc[state][haystack[i] - firstToken] == -1)
			state = failureFunc[state];

		state = gotoFunc[state][haystack[i] - firstToken];

		if (queryFunc[state])
			for (auto word : outputFunc.at(state))
				*output++ = std::make_pair(i - word.size() + 1, word);
	}
}

template <typename ChTy>
void AhoCorasickMachine<ChTy>::addWord(const StringType &word)
{
	size_t states = queryFunc.size();
	size_t currentState = 0;
	std::queue<size_t> queue;

	for (auto c : word)
	{
		if (c < firstToken || c > lastToken) throw std::runtime_error("Invalid letter");
		auto &nextState = gotoFunc[currentState][c - firstToken];
		if ((!currentState && !nextState) || nextState == -1) { //Si no hay un siguiente estado valido para este par {estado, caracter}, agregar un nuevo estado
			gotoFunc.emplace_back(lastToken - firstToken + 1, -1); //Construir una nueva fila del tama?o del abecedario, con -1 en todos los elementos
			nextState = states++;
		}

		currentState = gotoFunc[currentState][c - firstToken];
	}
	if (currentState >= queryFunc.size()) {
		queryFunc.resize(currentState + 1);
	}
	outputFunc[currentState].insert(word);
	queryFunc[currentState] = true;

	failureFunc.resize(states);

	for (ChTy i = 0, alphabetSize = lastToken - firstToken; i <= alphabetSize; ++i)
	{
		if (auto state = gotoFunc[0][i]) {
			queue.push(state);
			failureFunc[state] = 0;
		}
	}

	while (!queue.empty()) //armar funcion de fallo
	{
		auto r = queue.front();
		queue.pop();
		for (size_t a = 0; a < (size_t)(lastToken - firstToken); ++a)
		{
			auto s = gotoFunc[r][a];
			if (s != -1) {
				queue.push(s);
				auto state = failureFunc[r];
				while (gotoFunc[state][a] == -1)
					state = failureFunc[state];
				failureFunc[s] = gotoFunc[state][a];

				if (queryFunc[s] && queryFunc[failureFunc[s]]) {
					auto &embeddedWordSet = outputFunc.at(failureFunc[s]), &wordSet = outputFunc.at(s);

					wordSet.insert(embeddedWordSet.begin(), embeddedWordSet.end());
				}
			}
		}
	}
}

#endif