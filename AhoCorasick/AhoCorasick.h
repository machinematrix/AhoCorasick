#ifndef __AHOCORASICK__
#define __AHOCORASICK__
#include <vector>
#include <string>
#include <string_view>
#include <queue>
#include <map>
#include <set>
#include <stdexcept>
#include <iterator>

template <typename TokenType>
class AhoCorasickMachine
{
public:
	using StringType = std::basic_string<TokenType>;
	using StringViewType = std::basic_string_view<TokenType>;
	//<position, word>
	template <template<typename> typename Container>
	using FindResult = Container<std::pair<StringViewType, size_t>>;
private:
	const TokenType mFirstToken, mLastToken;
	std::vector<std::vector<size_t>> mGotoFunc; //rows: states | columns: letters in the range [firstToken;lastToken]
	std::map<size_t, std::set<StringType>> mOutputFunc;
	std::vector<size_t> mFailureFunc;
	std::vector<char> mQueryFunc; //tells us whether a state is an output state, without having to check outputFunc, which is slower.
public:
	AhoCorasickMachine(const TokenType firstToken = TokenType{}, const TokenType lastToken = std::numeric_limits<TokenType>::max());
	template <typename InputIterator>
	AhoCorasickMachine(InputIterator first, InputIterator last, const TokenType firstToken = TokenType{}, const TokenType lastToken = std::numeric_limits<TokenType>::max());
	AhoCorasickMachine(std::initializer_list<StringViewType> il, const TokenType mFirstToken = TokenType{}, const TokenType mLastToken = std::numeric_limits<TokenType>::max());
	template <typename OutputIterator>
	void find(StringViewType haystack, OutputIterator output) const;
	template <template<typename> typename Container = std::vector>
	FindResult<Container> find(StringViewType haystack) const;
	void addWord(StringViewType word);
};

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------

template <typename TokenType>
AhoCorasickMachine<TokenType>::AhoCorasickMachine(const TokenType firstToken, const TokenType lastToken)
	:mFirstToken(firstToken),
	mLastToken(lastToken),
	mGotoFunc(1, std::vector<size_t>(mLastToken - mFirstToken + 1, 0)),
	mFailureFunc(1, 0), //failure function can be accessed safely with a default initialized object
	mQueryFunc(1, false) //we don't also initialize outputFunc because this state is not an output state, and outputFunc contains only output states
{}

template <typename TokenType>
template <typename InputIterator>
AhoCorasickMachine<TokenType>::AhoCorasickMachine(InputIterator first, InputIterator last, const TokenType firstToken, const TokenType lastToken)
	:AhoCorasickMachine(firstToken, lastToken)
{
	if (lastToken <= firstToken)
		throw std::logic_error("Invalid alphabet");

	while (first != last) //make prefix tree
		addWord(*first++);
}

template <typename TokenType>
AhoCorasickMachine<TokenType>::AhoCorasickMachine(std::initializer_list<StringViewType> il, const TokenType firstToken, const TokenType lastToken)
	:AhoCorasickMachine(il.begin(), il.end(), firstToken, lastToken)
{}

template <typename TokenType>
template <typename OutputIterator>
void AhoCorasickMachine<TokenType>::find(StringViewType haystack, OutputIterator output) const
{
	size_t state = 0;

	for (typename StringType::size_type i = 0, sz = haystack.size(); i < sz; ++i)
	{
		while (mGotoFunc[state][haystack[i] - mFirstToken] == -1)
			state = mFailureFunc[state];

		state = mGotoFunc[state][haystack[i] - mFirstToken];

		if (mQueryFunc[state])
			for (const auto &word : mOutputFunc.at(state))
				*output++ = std::pair<StringViewType, size_t>(word, i - word.size() + 1);
	}
}

template<typename TokenType>
template<template<typename> typename Container>
AhoCorasickMachine<TokenType>::FindResult<Container> AhoCorasickMachine<TokenType>::find(StringViewType haystack) const
{
	FindResult<Container> result;

	find(haystack, std::inserter(result, result.end()));
	return result;
}

template <typename TokenType>
void AhoCorasickMachine<TokenType>::addWord(StringViewType word)
{
	size_t states = mQueryFunc.size(), currentState = 0;
	std::queue<size_t> queue;

	for (auto c : word)
	{
		if (c < mFirstToken || c > mLastToken)
			throw std::invalid_argument("Invalid token");

		auto &nextState = mGotoFunc[currentState][c - mFirstToken];

		if ((!currentState && !nextState) || nextState == -1) //Si no hay un siguiente estado valido para este par {estado, caracter}, agregar un nuevo estado
		{
			mGotoFunc.emplace_back(mLastToken - mFirstToken + 1, -1); //Construir una nueva fila del tama?o del abecedario, con -1 en todos los elementos
			nextState = states++;
		}

		currentState = mGotoFunc[currentState][c - mFirstToken];
	}

	if (currentState >= mQueryFunc.size())
		mQueryFunc.resize(currentState + 1);

	mOutputFunc[currentState].emplace(word.data());
	mQueryFunc[currentState] = true;

	mFailureFunc.resize(states);

	for (size_t i {}, alphabetSize = mLastToken - mFirstToken; i <= alphabetSize; ++i)
	{
		if (auto state = mGotoFunc[0][i])
		{
			queue.push(state);
			mFailureFunc[state] = 0;
		}
	}

	while (!queue.empty()) //assemble failure function
	{
		auto r = queue.front();

		queue.pop();

		for (size_t a = 0; a < static_cast<size_t>(mLastToken - mFirstToken); ++a)
		{
			if (auto s = mGotoFunc[r][a]; s != -1)
			{
				queue.push(s);
				auto state = mFailureFunc[r];
				while (mGotoFunc[state][a] == -1)
					state = mFailureFunc[state];

				mFailureFunc[s] = mGotoFunc[state][a];

				if (mQueryFunc[s] && mQueryFunc[mFailureFunc[s]])
				{
					auto &embeddedWordSet = mOutputFunc.at(mFailureFunc[s]), &wordSet = mOutputFunc.at(s);

					wordSet.insert(embeddedWordSet.begin(), embeddedWordSet.end());
				}
			}
		}
	}
}

#endif