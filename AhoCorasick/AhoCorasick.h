#ifndef __AHOCORASICK__
#define __AHOCORASICK__
#include <vector>
#include <string>
#include <iostream>
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
	std::vector<bool> queryFunc; //tells us whether a state is an output state, without having to check outputFunc, which is slower.
	//static inline void validate(const std::vector<std::pair<std::string::size_type, std::string>>::value_type&, const std::string&, size_t);
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
		//validate({ i, outputFunc[state] }, haystack, 1); //usado para testing
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
		if (c < firstToken || c > lastToken) throw std::runtime_error("Letra no valida");
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

//DON'T DELETE
//template <typename InputIterator>
//AhoCorasickMachine::AhoCorasickMachine(InputIterator first, InputIterator last, const unsigned char firstToken, const unsigned char lastToken)
//	:firstToken(firstToken),
//	lastToken(lastToken),
//	gotoFunc(1, std::vector<size_t>(lastToken - firstToken + 1, -1)),
//	queryFunc(1)
//{
//	if (lastToken <= firstToken) throw std::logic_error("Alfabeto invalido");
//	size_t states = 1;
//	std::queue<size_t> queue;
//
//	while (first != last) //armar arbol de prefijos
//	{
//		size_t currentState = 0;
//		for (auto c : *first)
//		{
//			if (c < firstToken || c > lastToken) throw std::runtime_error("Letra no valida");
//			if (gotoFunc[currentState][c - firstToken] == -1) { //Si no hay un siguiente estado valido para este par {estado, caracter}, agregar un nuevo estado
//				gotoFunc.emplace_back(lastToken - firstToken + 1, -1); //Construir una nueva fila del tama?o del abecedario, con -1 en todos los elementos
//				gotoFunc[currentState][c - firstToken] = states++;
//			}
//			currentState = gotoFunc[currentState][c - firstToken];
//		}
//		if (currentState >= queryFunc.size()) {
//			queryFunc.resize(currentState + 1);
//		}
//		outputFunc[currentState].insert(*first);
//		queryFunc[currentState] = true;
//
//		++first;
//	}
//
//	failureFunc.resize(states);
//
//	for (unsigned char i = 0, alphabetSize = lastToken - firstToken; i <= alphabetSize; ++i) { //poner en 0 todos los elementos no validos de la primera fila, porque el paper dice que g(0, x) nunca falla.
//		auto state = gotoFunc[0][i];
//		if (state == -1) gotoFunc[0][i] = 0;
//		else if (state) {
//			queue.push(state);
//			failureFunc[state] = 0;
//		}
//	}
//
//	while (!queue.empty()) //armar funcion de fallo
//	{
//		auto r = queue.front();
//		queue.pop();
//		for (size_t a = 0; a < (size_t)(lastToken - firstToken); ++a)
//		{
//			auto s = gotoFunc[r][a];
//			if (s != -1) {
//				queue.push(s);
//				auto state = failureFunc[r];
//				while (gotoFunc[state][a] == -1) state = failureFunc[state];
//				failureFunc[s] = gotoFunc[state][a];
//				
//				if (queryFunc[s] && queryFunc[failureFunc[s]]) {
//					auto &embeddedWordSet = outputFunc.at(failureFunc[s]), &wordSet = outputFunc.at(s);
//					
//					wordSet.insert(embeddedWordSet.begin(), embeddedWordSet.end());
//				}
//			}
//		}
//	}
//}

//void AhoCorasickMachine::validate(const std::vector<std::pair<std::string::size_type, std::string>>::value_type &result, const std::string &haystack, size_t interval)
//{
//	for (std::string::size_type size = result.second.size() - 1, i = size; i != -1; --i)
//	{
//		if (haystack[result.first - (size - i) * interval] != result.second[i]) {
//			std::cerr << result.second << ' ' << result.first << '\n';
//			break;
//		}
//	}
//	std::cerr.flush();
//}

#endif