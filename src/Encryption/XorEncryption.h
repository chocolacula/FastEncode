#pragma once

#include <stdint.h>

class XorEncryption final : public EncryptionBase
{
public:
	static void Encrypt(const uint8_t* input, uint32_t inputSize, uint8_t* output)
	{
		const auto d = 6081168334097007813;

		while (inputSize >= 64)
		{
			*(uint64_t*)output = *(uint64_t*)input ^ d;
			*(uint64_t*)(output + 8) = *(uint64_t*)(input + 8) ^ d;
			*(uint64_t*)(output + 16) = *(uint64_t*)(input + 16) ^ d;
			*(uint64_t*)(output + 24) = *(uint64_t*)(input + 24) ^ d;
			*(uint64_t*)(output + 32) = *(uint64_t*)(input + 32) ^ d;
			*(uint64_t*)(output + 40) = *(uint64_t*)(input + 40) ^ d;
			*(uint64_t*)(output + 48) = *(uint64_t*)(input + 48) ^ d;
			*(uint64_t*)(output + 56) = *(uint64_t*)(input + 56) ^ d;

			input += 64;
			output += 64;
			inputSize -= 64;
		}

		while (inputSize >= 32)
		{
			*(uint64_t*)output = *(uint64_t*)input ^ d;
			*(uint64_t*)(output + 8) = *(uint64_t*)(input + 8) ^ d;
			*(uint64_t*)(output + 16) = *(uint64_t*)(input + 16) ^ d;
			*(uint64_t*)(output + 24) = *(uint64_t*)(input + 24) ^ d;
			
			input += 32;
			output += 32;
			inputSize -= 32;
		}

		if (inputSize >= 16)
		{
			*(uint64_t*)output = *(uint64_t*)input ^ d;
			*(uint64_t*)(output + 8) = *(uint64_t*)(input + 8) ^ d;
			
			input += 16;
			output += 16;
			inputSize -= 16;
		}

		if (inputSize >= 8)
		{
			*(uint64_t*)output = *(uint64_t*)input ^ d;
			
			input += 8;
			output += 8;
			inputSize -= 8;
		}

		if (inputSize >= 4)
		{
			*(uint32_t*)output = *(uint32_t*)input ^ d;

			input += 4;
			output += 4;
			inputSize -= 4;
		}

		if (inputSize >= 2)
		{
			*(uint16_t*)output = *(uint16_t*)input ^ d;

			input += 2;
			output += 2;
			inputSize -= 2;
		}

		if (inputSize == 1)
			*output = *input ^ d;
	}

	static bool Decrypt(const uint8_t* encryptedData, uint32_t inputSize, uint8_t* clearData)
	{
		Encrypt(encryptedData, inputSize, clearData);

		return true;
	}
};