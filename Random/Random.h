#pragma once

#include <stdint.h>
#include <string>
#include <random>

enum class CharSet
{
	Numbers  = 1,
	Symbols  = 1 << 1,
	English  = 1 << 2,
	Chinese  = 1 << 3,
	Cyrillic = 1 << 4,
	All = Numbers | Symbols | English | Chinese | Cyrillic
};

inline CharSet operator| (CharSet lhs, CharSet rhs)
{
	return static_cast<CharSet>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline CharSet operator& (CharSet lhs, CharSet rhs)
{
	return static_cast<CharSet>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

class RandomGenerator
{
private:
	std::mt19937 _mt;
	std::uniform_int_distribution<uint32_t> _dist;

public:
	RandomGenerator(const std::mt19937 &mt, uint32_t from, uint32_t to)
	{
		_mt = mt;
		_dist = std::uniform_int_distribution<uint32_t>(from, to);
	};

	uint32_t Next()
	{
		return _dist(_mt);
	};
};

class Random
{
private:
	static std::random_device _rd;
	static std::mt19937 _mt;

	static const char16_t* NUMBERS;
	static const char16_t* SYMBOLS;
	static const char16_t* ENGLISH;
	static const char16_t* CHINESE;
	static const char16_t* CYRILLIC;

public:	
	static void RandomiseArray(char16_t* array, size_t length, CharSet chars);
	static void RandomiseArray(uint8_t* array, size_t length);
	static RandomGenerator GetGenerator(uint32_t from, uint32_t to);
};