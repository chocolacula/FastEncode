#include "Blowfish_default.h"
#include <memory.h>
#include <iostream>

#define CAST_64(v) *((uint64_t*)(v))
#define CAST_32(v) *((uint32_t*)(v))
#define CAST_16(v) *((uint16_t*)(v))

Blowfish_default::Blowfish_default(const uint8_t* password, uint32_t length, Reliability reliability)
{
	SetPassword(password, length);
	SetReliability(reliability);
}

void Blowfish_default::SetPassword(const uint8_t* password, uint32_t length)
{
	uint32_t key[16];

	for (uint32_t i = 0; i < 16 * 4; ++i)
	{
		auto j = i % length;
		((uint8_t*)key)[i] = password[j];
	}

	for (uint32_t i = 0; i < 16; ++i)
	{
		_pKey[i] ^= key[i];
	}

	_pKey[16] ^= key[0];
	_pKey[17] ^= key[1];

	uint8_t block[8];
	CAST_64(block) = 0;

	for (auto i = 0; i < 18; i += 2)
	{
		Encrypt64(block, block);
		CAST_64(_pKey + i) = CAST_64(block);
	}

	for (auto i = 0; i < 256; i += 2)
	{
		Encrypt64(block, block);
		CAST_64(_sBox1 + i) = CAST_64(block);
	}

	for (int i = 0; i < 256; i += 2)
	{
		Encrypt64(block, block);
		CAST_64(_sBox2 + i) = CAST_64(block);
	}

	for (int i = 0; i < 256; i += 2)
	{
		Encrypt64(block, block);
		CAST_64(_sBox3 + i) = CAST_64(block);
	}

	for (int i = 0; i < 256; i += 2)
	{
		Encrypt64(block, block);
		CAST_64(_sBox4 + i) = CAST_64(block);
	}
}

void Blowfish_default::SetReliability(Reliability reliability)
{
	auto r = static_cast<uint8_t>(reliability);

	_x32End = r;
	_x64End = r / 2;
}

void Blowfish_default::EncryptMain(const uint8_t*& clear, uint32_t clearSize, uint8_t*& encrypted)
{
	while (clearSize >= 16)
	{
		Encrypt128(clear, encrypted);

		clear += 16;
		encrypted += 16;
		clearSize -= 16;
	}

	if (clearSize >= 8)
	{
		Encrypt64(clear, encrypted);

		clear += 8;
		encrypted += 8;
	}
}

void Blowfish_default::DecryptMain(const uint8_t*& encrypted, uint32_t encryptedSize, uint8_t*& clear)
{
	while (encryptedSize >= 16)
	{
		Decrypt128(encrypted, clear);

		clear += 16;
		encrypted += 16;
		encryptedSize -= 16;
	}

	// if one block is left decrypt it
	if (encryptedSize == 8)
	{
		Decrypt64(encrypted, clear);

		clear += 8;
		encrypted += 8;
	}
}

void Blowfish_default::Encrypt128(const uint8_t* input, uint8_t* output)
{
	auto left = CAST_64(input + 8);
	auto right = CAST_64(input);

	auto p64Key = (uint64_t*)_pKey;

	uint64_t t;
	for (auto i = 0; i < _x64End - 1; ++i)
	{
		right ^= p64Key[i];
		left ^= F64(right);

		t = left;
		left = right;
		right = t;
	}

	t = left;
	left = right;
	right = t;

	left ^= p64Key[_x64End - 1];
	right ^= p64Key[_x64End];

	CAST_64(output + 8) = left;
	CAST_64(output) = right;
}

void Blowfish_default::Decrypt128(const uint8_t* input, uint8_t* output)
{
	auto left = CAST_64(input + 8);
	auto right = CAST_64(input);

	auto p64Key = (uint64_t*)_pKey;

	uint64_t t;
	for (auto i = _x64End; i > 1; --i)
	{
		right ^= p64Key[i];
		left ^= F64(right);

		t = left;
		left = right;
		right = t;
	}

	t = left;
	left = right;
	right = t;

	left ^= p64Key[1];
	right ^= p64Key[0];

	CAST_64(output + 8) = left;
	CAST_64(output) = right;
}

void Blowfish_default::Encrypt64(const uint8_t* input, uint8_t* output)
{
	auto left = CAST_32(input + 4);
	auto right = CAST_32(input);

	uint32_t t;
	for (auto i = 0; i < _x32End - 1; ++i)
	{
		right ^= _pKey[i];
		left  ^= F32(right);

		t = left;
		left  = right;
		right = t;
	}

	t = left;
	left  = right;
	right = t;

	left  ^= _pKey[_x32End - 1];
	right ^= _pKey[_x32End];

	CAST_32(output + 4) = left;
	CAST_32(output)	= right;
}

void Blowfish_default::Decrypt64(const uint8_t* input, uint8_t* output)
{
	auto left = CAST_32(input + 4);
	auto right = CAST_32(input);

    uint32_t t;     
    for (auto i = _x32End; i > 1; --i)
    {
		right ^= _pKey[i];
		left  ^= F32(right);
            
        t = left;
		left  = right;
		right = t;
    }
        
    t = left;
	left  = right;
	right = t;
        
	left  ^= _pKey[1];
	right ^= _pKey[0];

	CAST_32(output + 4) = left;
	CAST_32(output) = right;
}
    
void Blowfish_default::Encrypt32(const uint8_t* input, uint8_t* output)
{
	auto left = CAST_16(input + 2);
	auto right = CAST_16(input);

    uint16_t t;      
    for (auto i = 0; i < _x32End - 1; ++i)
    {
        right ^= (uint16_t)_pKey[i];
        left  ^= F16(right);
            
        t = left;
        left  = right;
        right = t;
    }
        
    t = left;
    left  = right;
    right = t;
        
    left  ^= (uint16_t)_pKey[_x32End - 1];
    right ^= (uint16_t)_pKey[_x32End];

	CAST_16(output + 2) = left;
	CAST_16(output) = right;
}
    
void Blowfish_default::Decrypt32(const uint8_t* input, uint8_t* output)
{
	auto left = CAST_16(input + 2);
	auto right = CAST_16(input);

	uint16_t t;     
    for (auto i = _x32End; i > 1; --i)
    {
        right ^= (uint16_t)_pKey[i];
        left  ^= F16(right);
            
        t = left;
        left  = right;
        right = t;
    }
        
    t = left;
    left  = right;
    right = t;
        
    left  ^= (uint16_t)_pKey[1];
    right ^= (uint16_t)_pKey[0];

	CAST_16(output + 2) = left;
	CAST_16(output) = right;
}

uint64_t Blowfish_default::F64(uint64_t block)
{
	auto p8 = (uint8_t*)& block;

	block = (((block >> 24) ^ block) & 0b0000000001111111000000000111111100000000011111110000000001111111);

	return ((((uint64_t*)_sBox1)[p8[0]] + ((uint64_t*)_sBox2)[p8[2]]) ^ ((uint64_t*)_sBox3)[p8[4]]) + ((uint64_t*)_sBox4)[p8[6]];
}

uint32_t Blowfish_default::F32(uint32_t block)
{      
    auto p8 = (uint8_t*) &block;
        
    return ((_sBox1[p8[0]] + _sBox2[p8[1]]) ^ _sBox3[p8[2]]) + _sBox4[p8[3]];
}
    
uint16_t Blowfish_default::F16(uint16_t block)
{
	auto p8 = (uint8_t*) &block;
        
    return _sBox1[p8[0]] + _sBox2[p8[1]];
} 