#include "gtest/gtest.h"

#include "../Random/Random.h"

#include "../src/Base64/Base64_default.h"
#include "../src/Base64/Architectures/AVX/Base64_avx.h"
#include "../src/Base64/Architectures/SSE/Base64_sse.h"
			 
#include "../src/Hashing/CRC32/Crc32c_default.h"
#include "../src/Hashing/CRC32/Architectures/SSE/Crc32c_sse.h"
#include "../src/Hashing/Fnv1a/Fnv1aYoshimitsu.h"
			 
#include "../src/Encryption/Blowfish/Blowfish_default.h"
#include "../src/Encryption/ChaCha20/chacha.h"

using namespace std::placeholders;

constexpr inline bool CompareArrays(const uint8_t* array1, const uint8_t* array2, uint32_t size)
{
	auto ok = true;
	for (uint32_t i = 0; i < size; ++i)
	{
		ok &= array1[i] == array2[i];
	}

	return ok;
}

inline void Process(std::function<void(uint32_t)> run)
{
	auto generator = Random::GetGenerator(1024, 4048);

	for (auto i = 0; i < 20; i++)
	{
		auto size = generator.Next();
		run(size);
	}
}

inline void ProcessBase64(std::function<void(const uint8_t*, uint32_t, char16_t*)> to,
				          std::function<void(const char16_t*, uint32_t, uint8_t*, uint32_t)> from)
{
	Process([to, from](uint32_t arrSize)
		{
			auto strSize = (arrSize / 3) * 4 + (arrSize % 3 > 0 ? 4 : 0);
			auto source = new uint8_t[arrSize];

			auto arr = new uint8_t[arrSize];
			auto str = new char16_t[strSize];

			Random::RandomiseArray(source, arrSize);
			memcpy_s(arr, arrSize, source, arrSize);

			to(arr, arrSize, str);
			Random::RandomiseArray(arr, arrSize);

			auto addition = 0;
			if (str[strSize - 1] == '=')
			{
				addition++;
				if (str[strSize - 2] == '=')
					addition++;
			}

			from(str, strSize, arr, addition);

			ASSERT_TRUE(CompareArrays(source, arr, arrSize));

			delete[] source;
			delete[] arr;
			delete[] str;
		});
}

inline void ProcessBlowfish(void (Blowfish_default::* to)(const uint8_t* clearData, uint32_t inputSize, uint8_t* encryptedData),
							bool (Blowfish_default::* from)(const uint8_t* encryptedData, uint32_t inputSize, uint8_t* clearData),
							Blowfish_default& blowfish)
{
	auto t = std::bind(to, blowfish, _1, _2, _3);
	auto f = std::bind(from, blowfish, _1, _2, _3);

	Process([&t, &f](uint32_t clearSize)
		{
			auto source = new uint8_t[clearSize];

			auto encrypted = new uint8_t[1 + clearSize + 8 - (clearSize % 8) + 4];
			auto decrypted = new uint8_t[clearSize];

			Random::RandomiseArray(source, clearSize);

			t(source, clearSize, encrypted);

			ASSERT_TRUE(f(encrypted, 1 + clearSize + 8 - encrypted[0] + 4, decrypted));
			ASSERT_TRUE(CompareArrays(source, decrypted, clearSize));

			delete[] source;
			delete[] encrypted;
			delete[] decrypted;
		});
}

inline void ProcessChaCha(uint32_t rounds, bool isX = false)
{
	Process([rounds, isX](uint32_t clearSize)
		{
			auto source = new uint8_t[clearSize];

			auto encrypted = new uint8_t[1 + clearSize + 8 - (clearSize % 8) + 4];
			auto decrypted = new uint8_t[clearSize];

			Random::RandomiseArray(source, clearSize);

			const chacha_key key = { {12, 42, 43, 47, 22, 14, 46, 28, 19, 34, 28, 26, 77, 68, 92, 34, 78, 33, 45, 123, 15, 79, 32, 28, 23, 49, 64, 75, 82, 203, 48, 31} };
			const chacha_iv iv = { {55, 32, 14, 76, 48, 99, 123, 15} };
			const chacha_iv24 iv24 = { {55, 32, 14, 76, 48, 99, 123, 15, 46, 35, 126, 63, 83, 36, 14, 62, 48, 24, 71, 93, 47, 109, 31, 53} };

			if (isX)
			{
				xchacha(&key, &iv24, source, encrypted, clearSize, rounds);
				xchacha(&key, &iv24, encrypted, decrypted, clearSize, rounds);
			}
			else
			{
				chacha(&key, &iv, source, encrypted, clearSize, rounds);
				chacha(&key, &iv, encrypted, decrypted, clearSize, rounds);
			}

			ASSERT_TRUE(CompareArrays(source, decrypted, clearSize));

			delete[] source;
			delete[] encrypted;
			delete[] decrypted;
		});
}

TEST(Base64, Default)
{
	ProcessBase64(Default::ToBase64, Default::FromBase64);
}

TEST(Base64, AVX2)
{
	ProcessBase64(Base64_avx::ToBase64, Base64_avx::FromBase64);
}

TEST(Base64, SSE)
{
	ProcessBase64(Base64_sse::ToBase64, Base64_sse::FromBase64);
}

TEST(Crc32c, Equal)
{
	Process([](uint32_t arrSize)
		{
			auto arr = new uint8_t[arrSize];

			Random::RandomiseArray(arr, arrSize);

			auto a = Crc32c_default::Calculate(arr, arrSize);
			auto b = Crc32c_sse::Calculate(arr, arrSize);

			ASSERT_TRUE(a == b);

			delete[] arr;
		});
}

TEST(Fnv1aYoshimitsu, Append32)
{
	Process([](uint32_t arrSize)
		{
			auto arr = new uint8_t[arrSize];

			Random::RandomiseArray(arr, arrSize);

			auto a = Fnv1aYoshimitsu::Calculate(arr, arrSize);

			Fnv1aYoshimitsu::State state(arr);

			auto length = arrSize;
			for (; length >= 32; length -= 32)
			{
				Fnv1aYoshimitsu::Append32(state);
			}

			auto b = Fnv1aYoshimitsu::End(state, length);

			ASSERT_TRUE(a == b);

			delete[] arr;
		});
}

TEST(Blowfish, Strong)
{
	auto password = "ololoatata";
	Blowfish_default blowfish((uint8_t*)password, sizeof(password) - 1, Blowfish_default::Reliability::Strong);

	ProcessBlowfish(&Blowfish_default::Encrypt, &Blowfish_default::Decrypt, blowfish);
}

TEST(Blowfish, Medium)
{
	auto password = "ololoatata";
	Blowfish_default blowfish((uint8_t*)password, sizeof(password) - 1, Blowfish_default::Reliability::Medium);

	ProcessBlowfish(&Blowfish_default::Encrypt, &Blowfish_default::Decrypt, blowfish);
}

TEST(Blowfish, Weak)
{
	auto password = "ololoatata";
	Blowfish_default blowfish((uint8_t*)password, sizeof(password) - 1, Blowfish_default::Reliability::Weak);

	ProcessBlowfish(&Blowfish_default::Encrypt, &Blowfish_default::Decrypt, blowfish);
}

TEST(ChaCha, 20)
{
	ProcessChaCha(20);
}

TEST(ChaCha, 12)
{
	ProcessChaCha(12);
}

TEST(ChaCha, 8)
{
	ProcessChaCha(8);
}

TEST(ChaCha, X20)
{
	ProcessChaCha(20, true);
}

TEST(ChaCha, X12)
{
	ProcessChaCha(12, true);
}

TEST(ChaCha, X8)
{
	ProcessChaCha(8, true);
}