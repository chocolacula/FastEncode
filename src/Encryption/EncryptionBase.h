#pragma once

#include <stdint.h>
#include <functional>
#include "../Hashing/Crc32/Architectures/SSE/Crc32c_sse.h"

class EncryptionBase
{
private:
	std::function<uint32_t(const uint8_t*, uint32_t)> _hashFunction = std::bind(&Crc32c_sse::Calculate, std::placeholders::_1, std::placeholders::_2, 0xFFFFFFFF);

protected:
	virtual inline void EncryptMain(const uint8_t*& clear, uint32_t clearSize, uint8_t*& encrypted) = 0;
	virtual inline void DecryptMain(const uint8_t*& encrypted, uint32_t encryptedSize, uint8_t*& clear) = 0;

	virtual inline void Encrypt64(const uint8_t* clear, uint8_t* encrypted) = 0;
	virtual inline void Decrypt64(const uint8_t* encrypted, uint8_t* clear) = 0;

	virtual inline void Encrypt32(const uint8_t* clear, uint8_t* encrypted) = 0;
	virtual inline void Decrypt32(const uint8_t* encrypted, uint8_t* clear) = 0;

public:
	virtual ~EncryptionBase() 
	{}

	void SetHash(std::function<uint32_t(const uint8_t*, uint32_t)> hashFunction)
	{
		_hashFunction = hashFunction;
	} 

	void Encrypt(const uint8_t* clear, uint32_t clearSize, uint8_t* encrypted)
	{
		// write rest size in the beginning of data
		uint8_t rest = clearSize % 8;
		*encrypted++ = rest;

		EncryptMain(clear, clearSize - rest, encrypted);

		// zeroing tail
		*(uint64_t*)encrypted = 0;

		// copy rest to destination
		memcpy_s(encrypted, rest, clear, rest);

		// encrypt rest, the pointer will be increased later
		Encrypt64(encrypted, encrypted);

		/* sequential hash calculation after whole encryption is slightly faster then simultaneously calculation
		 * it can be explained by less amount of POP/PUSH stack operations and more effective registers using without extra MOV
		 * these results are obtained for encrypting of 3000002 bytes with SSE version of Crc32c
		 * --------------------------------------------------------------------
		 * Benchmark                          Time             CPU   Iterations
		 * --------------------------------------------------------------------
		 * Blowfish_encryptAll         20594461 ns     20507813 ns           32
		 * Blowfish_encryptBlock       27404739 ns     27644231 ns           26
		 */		

		// calculate hash of all encrypted data
		auto hash = _hashFunction(encrypted - clearSize + rest, clearSize - rest + 8);
		
		// increase the pointer
		encrypted += 8;	

		// encrypt hash sum too and write then
		Encrypt32((uint8_t*)&hash, encrypted);
	}

	bool Decrypt(const uint8_t* encrypted, uint32_t encryptedSize, uint8_t* clear)
	{
		// calculate hash sum of all encrypted data
		// 5 = a byte for the rest size + 4 bytes for the hash 
		auto hash1 = _hashFunction(encrypted + 1, encryptedSize - 5);

		uint32_t hash2;

		// decrypt hash sum stored in data
		Decrypt32(encrypted + encryptedSize - 4, (uint8_t*)& hash2);

		if (hash1 != hash2)
			return false;

		// starting from begin again, get rest 
		auto rest = *encrypted++;

		DecryptMain(encrypted, encryptedSize - 8 - 5, clear);

		uint8_t tail[8];

		// decrypt last block of data and store to temp uint8_t array
		Decrypt64(encrypted, tail);

		// copy rest from tail to destination
		memcpy_s(clear, rest, tail, rest);

		return true;
	}
};
