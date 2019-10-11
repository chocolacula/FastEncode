#pragma once

#include <stdint.h>

class Default
{
private:	
	static inline char16_t GetSymbol(uint8_t index)
	{
		//static const uint8_t SYMBOL_TABLE[] = {65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 43, 47};
		static const char16_t* SYMBOL_TABLE = u"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		return SYMBOL_TABLE[index];
	}

	static inline uint8_t GetIndex(char16_t symbol)
	{
		static const uint8_t INDEX_TABLE[] = {62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
		return INDEX_TABLE[symbol - 43];
	}

	static void Encrypt32(const uint8_t* input, char16_t* output);
	static void Decrypt32(const char16_t* input, uint8_t* output);
	static void Decrypt24(const char16_t* input, uint8_t* output);
	static void EncryptTail(const uint8_t* input, char16_t* output, uint32_t tailSize);
	static void DecryptTail(const char16_t* input, uint8_t* output, uint32_t tailSize);

public:
	static void ToBase64(const uint8_t* input, uint32_t inputSize, char16_t* output);
	static void FromBase64(const char16_t* input, uint32_t inputSize, uint8_t* output, uint32_t addition);
};