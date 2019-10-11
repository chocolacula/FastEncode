#include "Base64_default.h"
#include <stdlib.h>

void Default::Encrypt32(const uint8_t* input, char16_t* output)
{
	// | b7 b6 b5 b4 b3 b2 b1 b0 | a7 a6 a5 a4 a3 a2 a1 a0 |
	// in this case with little-endian order to match classic RFC 4648 version 
	// without URL safe chars and new lines every 76 character like in MIME
	// we need 6 high digits bits of a, 2 low and 4 high digits of b etc.
	// it little simpler with big-endian order

	auto block = _byteswap_ulong(*(uint32_t*)input);

	*output++ = GetSymbol(block >> 26);
	*output++ = GetSymbol((block >> 20) & 0x3F); // 0x3F = 00111111
	*output++ = GetSymbol((block >> 14) & 0x3F);
	*output++ = GetSymbol((block >> 8) & 0x3F);
}

void Default::Decrypt32(const char16_t* input, uint8_t* output)
{
	uint32_t block;

	block = GetIndex(*input) << 26;
	block |= GetIndex(*(input + 1)) << 20;
	block |= GetIndex(*(input + 2)) << 14;
	block |= GetIndex(*(input + 3)) << 8;

	block = _byteswap_ulong(block);
	*(uint32_t*)output = block;

	output += 3;
}

void Default::Decrypt24(const char16_t* input, uint8_t* output)
{
	*output++ = (GetIndex(*input) << 2) | (GetIndex(*(input + 1)) >> 4);
	*output++ = ((GetIndex(*(input + 1)) & 0x0f) << 4) | (GetIndex(*(input + 2)) >> 2);
	*output++ = ((GetIndex(*(input + 2)) & 0x03) << 6) | GetIndex(*(input + 3));
}

void Default::EncryptTail(const uint8_t* input, char16_t* output, uint32_t tailSize)
{
	if (tailSize == 0)
		return;
	else
		*output++ = GetSymbol(*input >> 2);

	if (tailSize == 1)
	{
		*output++ = GetSymbol((*input & 3) << 4);
		*output++ = '=';
		*output++ = '=';
	}
	else if (tailSize == 2)
	{
		*output++ = GetSymbol(((*input & 3) << 4) | (*(input + 1) >> 4));
		*output++ = GetSymbol((*(input + 1) & 0x0F) << 2);
		*output++ = '=';
	}
}

void Default::DecryptTail(const char16_t* input, uint8_t* output, uint32_t tailSize)
{
	if (tailSize != 0)
		*output++ = (GetIndex(*input) << 2) | (GetIndex(*(input + 1)) >> 4);

	if (tailSize == 1)
		*output++ = ((GetIndex(*(input + 1)) & 0x0f) << 4) | (GetIndex(*(input + 2)) >> 2);
}


void Default::ToBase64(const uint8_t* input, uint32_t inputSize, char16_t* output)
{
	if (inputSize == 0)
		return;

	if (inputSize < 3)
	{
		EncryptTail(input, output, inputSize);
		return;
	}

	auto rest = inputSize % 3;
	auto limit = rest == 0 ? inputSize : inputSize - rest;

	for (uint32_t i = 0; i < limit; i += 3)
	{
		Encrypt32(input, output);

		input += 3;
		output += 4;
	}

	EncryptTail(input, output, inputSize - limit);
}

void Default::FromBase64(const char16_t* input, uint32_t inputSize, uint8_t* output, uint32_t addition)
{
	if (inputSize == 0)
		return;

	if (inputSize < 8)
	{
		if (inputSize < 4)
		{
			DecryptTail(input, output, inputSize);
			return;
		}

		if (inputSize == 4)
		{
			Decrypt24(input, output);
			return;
		}

		if (inputSize > 4)
		{
			Decrypt24(input, output);
			DecryptTail(input + 4, output + 3, inputSize);
			return;
		}
	}

	auto limit = (addition == 0 ? inputSize : inputSize - 4) - 4;

	for (uint32_t i = 0; i < limit; i += 4)
	{
		Decrypt32(input, output);

		input += 4;
		output += 3;
	}

	Decrypt24(input, output);
	DecryptTail(input + 4, output + 3, addition);
}