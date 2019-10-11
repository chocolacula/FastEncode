#include "Base64_avx.h"

void Base64_avx::ToBase64(const uint8_t* input, uint32_t inputSize, char16_t* output)
{
	auto rest = inputSize % 24;
	auto limit = (rest == 0 ? inputSize : inputSize - rest) - 24;

	uint32_t i = 0;
	for (; i < limit; i += 24)
	{
		auto inputvector = _mm256_loadu_si256((__m256i*)input);

		inputvector = EncryptReshuffle(inputvector);
		inputvector = EncryptTranslate(inputvector);

		inputvector = _mm256_permutevar8x32_epi32(inputvector, _mm256_set_epi32(7, 6, 3, 2, 5, 4, 1, 0));

		const auto left = _mm256_set_epi8(-1, 7, -1, 6, -1, 5, -1, 4, -1, 3, -1, 2, -1, 1, -1, 0,
										  -1, 7, -1, 6, -1, 5, -1, 4, -1, 3, -1, 2, -1, 1, -1, 0);

		const auto right = _mm256_set_epi8(-1, 15, -1, 14, -1, 13, -1, 12, -1, 11, -1, 10, -1, 9, -1, 8, 
										   -1, 15, -1, 14, -1, 13, -1, 12, -1, 11, -1, 10, -1, 9, -1, 8);

		auto v1 = _mm256_shuffle_epi8(inputvector, left);
		auto v2 = _mm256_shuffle_epi8(inputvector, right);

		_mm256_storeu_si256((__m256i*)output,     v1);
		_mm256_storeu_si256((__m256i*)output + 1, v2);

		input += 24;
		output += 32;
	}

	Default::ToBase64(input, inputSize - i, output);
}

void Base64_avx::FromBase64(const char16_t* input, uint32_t inputSize, uint8_t* output, uint32_t addition)
{
	auto rest = inputSize % 32;
	auto limit = (rest == 0 ? inputSize : inputSize - rest) - 32;
		
	uint32_t i = 0;
	for (; i < limit; i += 32)
	{
		auto leftVector  = _mm256_loadu_si256((__m256i*)input);
		auto rightVector = _mm256_loadu_si256((__m256i*)input + 1);

		const auto leftMask = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0,
											  -1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0);

		const auto rightMask = _mm256_set_epi8(14, 12, 10, 8, 6, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1, -1,
											   14, 12, 10, 8, 6, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1, -1);

		leftVector  = _mm256_shuffle_epi8(leftVector, leftMask);
		rightVector = _mm256_shuffle_epi8(rightVector, rightMask);

		auto inputvector = _mm256_or_si256(leftVector, rightVector);

		inputvector = _mm256_permutevar8x32_epi32(inputvector, _mm256_set_epi32(7, 6, 3, 2, 5, 4, 1, 0));

		inputvector = DecryptTranslate(inputvector);
		inputvector = DecryptReshuffle(inputvector);

		_mm256_storeu_si256((__m256i*)output, inputvector);

		input += 32;
		output += 24;
	}

	Default::FromBase64(input, inputSize - i, output, addition);
}