#pragma comment (lib, "Shlwapi.lib")

#include <iostream>
#include <functional>
#include <chrono>
#include "benchmark/benchmark.h"

#include "../Random/Random.h"

#include "../src/Base64/Base64_default.h"
#include "../src/Base64/Architectures/AVX/Base64_avx.h"
#include "../src/Base64/Architectures/SSE/Base64_sse.h"
			 
#include "../src/Hashing/XorHash.h"
#include "../src/Hashing/CRC32/Crc32c_default.h"
#include "../src/Hashing/CRC32/Architectures/SSE/Crc32c_sse.h"
#include "../src/Hashing/Highway/Highway_default.h"
#include "../src/Hashing/Fnv1a/Fnv1aYoshimitsu.h"
			 
#include "../src/Encryption/Blowfish/Blowfish_default.h"
#include "../src/Encryption/ChaCha20/chacha.h"
#include "../src/Encryption/XorEncryption.h"

const auto ARR_SIZE = 3000002;
const auto STR_SIZE = (ARR_SIZE / 3) * 4 + (ARR_SIZE % 3 > 0 ? 4 : 0);
const auto BLOWFISH_SIZE = 1 + ARR_SIZE + 8 - (ARR_SIZE % 8) + 4;

uint8_t*  arr;
char16_t* str;

uint8_t* encrypted;
uint8_t* decrypted;

const chacha_key key = { {12, 42, 43, 47, 22, 14, 46, 28, 19, 34, 28, 26, 77, 68, 92, 34, 78, 33, 45, 123, 15, 79, 32, 28, 23, 49, 64, 75, 82, 203, 48, 31} };
const chacha_iv iv = { {55, 32, 14, 76, 48, 99, 123, 15} };
const chacha_iv24 iv24 = { {55, 32, 14, 76, 48, 99, 123, 15, 46, 35, 126, 63, 83, 36, 14, 62, 48, 24, 71, 93, 47, 109, 31, 53} };

void Base64_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		Default::ToBase64(arr, ARR_SIZE, str);
}

void Base64_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		Default::FromBase64(str, STR_SIZE, arr, 0);
}

void Base64_avx_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		Base64_avx::ToBase64(arr, ARR_SIZE, str);
}

void Base64_avx_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		Base64_avx::FromBase64(str, STR_SIZE, arr, 0);
}

void Base64_sse_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		Base64_sse::ToBase64(arr, ARR_SIZE, str);
}

void Base64_sse_decrypt(benchmark::State & state)
{
	for (auto _ : state)
		Base64_sse::FromBase64(str, STR_SIZE, arr, 0);
}

void XorHash(benchmark::State& state)
{
	for (auto _ : state)
		XorHash::Calculate(arr, ARR_SIZE);
}

void Crc32_default(benchmark::State& state)
{
	for (auto _ : state)
		Crc32c_default::Calculate(arr, ARR_SIZE);
}

void Crc32_sse(benchmark::State& state)
{
	for (auto _ : state)
		Crc32c_sse::Calculate(arr, ARR_SIZE);
}

void Highway_default(benchmark::State& state)
{
	uint64_t key[4] = { 352, 497, 184, 716 };

	for (auto _ : state)
		Highway_default::HighwayHash64(arr, ARR_SIZE, key);
}

void Fnv1aYoshimitsuTriad(benchmark::State& state)
{
	for (auto _ : state)
		Fnv1aYoshimitsu::CalculateTriad(arr, ARR_SIZE);
}

void Fnv1aYoshimitsu(benchmark::State& state)
{
	for (auto _ : state)
		Fnv1aYoshimitsu::Calculate(arr, ARR_SIZE);
}

void XorEncryption_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		XorEncryption::Encrypt(arr, ARR_SIZE, encrypted);
}

void XorEncryption_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		XorEncryption::Decrypt(encrypted, BLOWFISH_SIZE, decrypted);
}

void Blowfish_default_Strong_encrypt(benchmark::State& state)
{
	Blowfish_default blowfish((uint8_t*)"0123456789", 10, Blowfish_default::Reliability::Strong);

	for (auto _ : state)
		blowfish.Encrypt(arr, ARR_SIZE, encrypted);
}

void Blowfish_default_Strong_decrypt(benchmark::State& state)
{
	Blowfish_default blowfish((uint8_t*)"0123456789", 10, Blowfish_default::Reliability::Strong);

	for (auto _ : state)
		blowfish.Decrypt(encrypted, BLOWFISH_SIZE, decrypted);
}

void Blowfish_default_Strong_decryptCorrupted(benchmark::State& state)
{
	Blowfish_default blowfish((uint8_t*)"0123456789", 10, Blowfish_default::Reliability::Strong);

	auto pCrc = (uint32_t*) &encrypted[BLOWFISH_SIZE - 4];

	auto crc = *pCrc;
	*pCrc = 0;

	for (auto _ : state)
		blowfish.Decrypt(encrypted, BLOWFISH_SIZE, decrypted);

	*pCrc = crc;
}

void Blowfish_default_Medium_encrypt(benchmark::State& state)
{
	Blowfish_default blowfish((uint8_t*)"0123456789", 10, Blowfish_default::Reliability::Medium);

	for (auto _ : state)
		blowfish.Encrypt(arr, ARR_SIZE, encrypted);
}

void Blowfish_default_Medium_decrypt(benchmark::State& state)
{
	Blowfish_default blowfish((uint8_t*)"0123456789", 10, Blowfish_default::Reliability::Medium);

	for (auto _ : state)
		blowfish.Decrypt(encrypted, BLOWFISH_SIZE, decrypted);
}

void Blowfish_default_Weak_encrypt(benchmark::State& state)
{
	Blowfish_default blowfish((uint8_t*)"0123456789", 10, Blowfish_default::Reliability::Weak);

	for (auto _ : state)
		blowfish.Encrypt(arr, ARR_SIZE, encrypted);
}

void Blowfish_default_Weak_decrypt(benchmark::State& state)
{
	Blowfish_default blowfish((uint8_t*)"0123456789", 10, Blowfish_default::Reliability::Weak);

	for (auto _ : state)
		blowfish.Decrypt(encrypted, BLOWFISH_SIZE, decrypted);
}

void ChaCha20_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		chacha(&key, &iv, arr, encrypted, ARR_SIZE, 20);
}

void ChaCha20_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		chacha(&key, &iv, encrypted, decrypted, BLOWFISH_SIZE, 20);
}

void ChaCha12_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		chacha(&key, &iv, arr, encrypted, ARR_SIZE, 12);
}

void ChaCha12_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		chacha(&key, &iv, encrypted, decrypted, BLOWFISH_SIZE, 12);
}

void ChaCha8_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		chacha(&key, &iv, arr, encrypted, ARR_SIZE, 8);
}

void ChaCha8_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		chacha(&key, &iv, encrypted, decrypted, BLOWFISH_SIZE, 8);
}

void XChaCha20_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		xchacha(&key, &iv24, arr, encrypted, ARR_SIZE, 20);
}

void XChaCha20_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		xchacha(&key, &iv24, encrypted, decrypted, BLOWFISH_SIZE, 20);
}

void XChaCha12_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		xchacha(&key, &iv24, arr, encrypted, ARR_SIZE, 12);
}

void XChaCha12_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		xchacha(&key, &iv24, encrypted, decrypted, BLOWFISH_SIZE, 12);
}

void XChaCha8_encrypt(benchmark::State& state)
{
	for (auto _ : state)
		xchacha(&key, &iv24, arr, encrypted, ARR_SIZE, 8);
}

void XChaCha8_decrypt(benchmark::State& state)
{
	for (auto _ : state)
		xchacha(&key, &iv24, encrypted, decrypted, BLOWFISH_SIZE, 8);
}

BENCHMARK(Base64_encrypt);
BENCHMARK(Base64_decrypt);
BENCHMARK(Base64_avx_encrypt);
BENCHMARK(Base64_avx_decrypt);
BENCHMARK(Base64_sse_encrypt);
BENCHMARK(Base64_sse_decrypt);

BENCHMARK(XorHash);
BENCHMARK(Crc32_default);
BENCHMARK(Crc32_sse);
BENCHMARK(Highway_default);
BENCHMARK(Fnv1aYoshimitsuTriad);
BENCHMARK(Fnv1aYoshimitsu);

BENCHMARK(XorEncryption_encrypt);
BENCHMARK(XorEncryption_decrypt);

BENCHMARK(Blowfish_default_Strong_encrypt);
BENCHMARK(Blowfish_default_Strong_decrypt);
BENCHMARK(Blowfish_default_Strong_decryptCorrupted);
BENCHMARK(Blowfish_default_Medium_encrypt);
BENCHMARK(Blowfish_default_Medium_decrypt);
BENCHMARK(Blowfish_default_Weak_encrypt);
BENCHMARK(Blowfish_default_Weak_decrypt);

BENCHMARK(ChaCha20_encrypt);
BENCHMARK(ChaCha20_decrypt);
BENCHMARK(ChaCha12_encrypt);
BENCHMARK(ChaCha12_decrypt);
BENCHMARK(ChaCha8_encrypt);
BENCHMARK(ChaCha8_decrypt);
BENCHMARK(XChaCha20_encrypt);
BENCHMARK(XChaCha20_decrypt);
BENCHMARK(XChaCha12_encrypt);
BENCHMARK(XChaCha12_decrypt);
BENCHMARK(XChaCha8_encrypt);
BENCHMARK(XChaCha8_decrypt);

int main(int argc, char** argv)
{
	arr = new uint8_t[ARR_SIZE];
	str = new char16_t[STR_SIZE];

	encrypted = new uint8_t[BLOWFISH_SIZE];
	decrypted = new uint8_t[ARR_SIZE];

	Random::RandomiseArray(arr, ARR_SIZE);
	Random::RandomiseArray(str, STR_SIZE, CharSet::English | CharSet::Numbers);

	benchmark::Initialize(&argc, argv);
	benchmark::RunSpecifiedBenchmarks();

	delete[] arr;
	delete[] str;
}