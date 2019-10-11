#include "Base64_sse.h"

void Base64_sse::ToBase64(const uint8_t* input, uint32_t inputSize, char16_t* output)
{
	auto rest = inputSize % 12;
	auto limit = (rest == 0 ? inputSize : inputSize - rest) - 12;

	uint32_t i = 0;
	for (; i < limit; i += 12)
	{
		auto inputvector = _mm_loadu_si128((__m128i*)input);

		inputvector = EncryptReshuffle(inputvector);
		inputvector = EncryptTranslate(inputvector);

		const auto left  = _mm_set_epi8(-1, 7, -1, 6, -1, 5, -1, 4, -1, 3, -1, 2, -1, 1, -1, 0);
		const auto right = _mm_set_epi8(-1, 15, -1, 14, -1, 13, -1, 12, -1, 11, -1, 10, -1, 9, -1, 8);

		auto v1 = _mm_shuffle_epi8(inputvector, left);
		auto v2 = _mm_shuffle_epi8(inputvector, right);

		_mm_storeu_si128((__m128i*)output, v1);
		_mm_storeu_si128((__m128i*)output + 1, v2);

		input += 12;
		output += 16;
	}

	Default::ToBase64(input, inputSize - i, output);
}

void Base64_sse::FromBase64(const char16_t* input, uint32_t inputSize, uint8_t* output, uint32_t addition)
{
	auto rest = inputSize % 16;
	auto limit = (rest == 0 ? inputSize : inputSize - rest) - 16;

	uint32_t i = 0;
	for (; i < limit; i += 16)
	{
		auto leftVector = _mm_loadu_si128((__m128i*)input);
		auto rightVector = _mm_loadu_si128((__m128i*)input + 1);

		const auto leftMask  = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0);
		const auto rightMask = _mm_set_epi8(14, 12, 10, 8, 6, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1, -1);

		leftVector = _mm_shuffle_epi8(leftVector, leftMask);
		rightVector = _mm_shuffle_epi8(rightVector, rightMask);

		auto inputvector = _mm_or_si128(leftVector, rightVector);

		inputvector = DecryptTranslate(inputvector);
		inputvector = DecryptReshuffle(inputvector);

		_mm_storeu_si128((__m128i*)output, inputvector);

		input += 16;
		output += 12;
	}

	Default::FromBase64(input, inputSize - i, output, addition);
}