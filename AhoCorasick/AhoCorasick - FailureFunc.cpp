#include "AhoCorasick.h"
#include <iostream>
#include <queue>

/*
state <- 0;
for i until n do
{
	while g(state, a[i]) == fail do state <- f(state);

	state <- g(state, a[i]);

	if output(state) != empty then
	{
		print i;
		print output(state);
	}
}
*/

//std::vector<size_t> AhoCorasickMachine::makeHeightMap() const
//{
//	std::vector<size_t> result(gotoFunc.rowCount(), 0);
//	std::queue<size_t> queue;
//
//	for (size_t i = 0; i <= lastToken - firstToken; ++i) {
//		if (auto state = gotoFunc(0, i)) {
//			result[state] = 1;
//			queue.push(state);
//		}
//	}
//
//	while (!queue.empty())
//	{
//		auto nextState = queue.front();
//		queue.pop();
//		for (auto i = 0; i <= lastToken - firstToken; ++i)
//		{
//			if (gotoFunc(nextState, i) != -1) {
//				result[gotoFunc(nextState, i)] = result[nextState] + 1;
//				queue.push(gotoFunc(nextState, i));
//			}
//		}
//	}
//	return result;
//}

void AhoCorasickMachine::validate(const std::vector<std::pair<std::string::size_type, std::string>>::value_type &result, const std::string &haystack, size_t interval)
{
	for (std::string::size_type size = result.second.size() - 1, i = size; i != -1; --i)
	{
		if (haystack[result.first - (size - i) * interval] != result.second[i]) {
			std::cerr << result.second << ' ' << result.first << '\n';
			break;
		}
	}
	std::cerr.flush();
}

AhoCorasickMachine::AhoCorasickMachine(const std::vector<std::string> &words, const char firstToken, const char lastToken)
	:firstToken(firstToken),
	 lastToken(lastToken),
	 gotoFunc(1, std::vector<size_t>(lastToken - firstToken + 1, -1)),
	 outputFunc(1) //primer output siempre va a ser el string vacio
{
	if (lastToken <= firstToken) throw std::logic_error("Alfabeto invalido");
	size_t states = 1;
	std::queue<size_t> queue;

	for (const auto &word : words) //armar arbol de prefijos
	{
		size_t currentState = 0;
		for (auto c : word)
		{
			if (gotoFunc[currentState][c - firstToken] == -1) { //Si no hay un siguiente estado valido para este par (estado, caracter), agregar un nuevo estado
				gotoFunc.emplace_back(lastToken - firstToken + 1, -1); //Construir una nueva fila del tama?o del abecedario, con -1 en todos los elementos
				gotoFunc[currentState][c - firstToken] = states++;
			}
			currentState = gotoFunc[currentState][c - firstToken];
		}
		if (currentState >= outputFunc.size()) { //si, es un desperdicio de memoria, pero es mas rapido que llamar find en un map tantas veces como caracteres haya	
			outputFunc.resize(currentState + 1);
		}
		outputFunc[currentState] = word;
	}

	failureFunc.resize(states);

	for (char i = 0; i <= lastToken - firstToken; ++i) { //poner en 0 todos los elementos no validos de la primera fila, porque el paper dice que g(0, x) nunca falla.
		auto state = gotoFunc[0][i];
		if (state == -1)
			gotoFunc[0][i] = 0;
		else if (state) {
			queue.push(state);
			failureFunc[state] = 0;
		}
	}

	while (!queue.empty()) //armar funcion de fallo
	{
		auto r = queue.front();
		queue.pop();
		for (size_t a = 0; a < lastToken - firstToken; ++a)
		{
			auto s = gotoFunc[r][a];
			if (s != -1) {
				queue.push(s);
				auto state = failureFunc[r];
				while (gotoFunc[state][a] == -1) state = failureFunc[state];
				failureFunc[s] = gotoFunc[state][a];
			}
		}
	}
}

std::vector<std::pair<std::string::size_type, std::string>> AhoCorasickMachine::find(const std::string &haystack) const
{
	std::vector<std::pair<std::string::size_type, std::string>> result;
	size_t state = 0;

	for (std::string::size_type i = 0; i < haystack.size(); ++i)
	{
		while (gotoFunc[state][haystack[i] - firstToken] == -1)
			state = failureFunc[state];
		state = gotoFunc[state][haystack[i] - firstToken];

		if (!outputFunc[state].empty())
			result.emplace_back(i, outputFunc[state]);
	}

	return result;
}