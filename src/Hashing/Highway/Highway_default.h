#pragma once

#include <stdint.h>
#include <string.h>

class Highway_default
{
public:
	static uint32_t HighwayHash32(const uint8_t* data, size_t size, const uint64_t key[4]);
	static uint64_t HighwayHash64(const uint8_t* data, size_t size, const uint64_t key[4]);
	static void     HighwayHash128(const uint8_t* data, size_t size, const uint64_t key[4], uint64_t hash[2]);
	static void     HighwayHash256(const uint8_t* data, size_t size, const uint64_t key[4], uint64_t hash[4]);

private:
	typedef struct {
		uint64_t v0[4];
		uint64_t v1[4];
		uint64_t mul0[4];
		uint64_t mul1[4];
	} HighwayHashState;

	typedef struct {
		HighwayHashState state;
		uint8_t packet[32];
		int num;
	} HighwayHashCat;

	/* Initializes state with given key */
	static void HighwayHashReset(const uint64_t key[4], HighwayHashState* state);

	static void ZipperMergeAndAdd(const uint64_t v1, const uint64_t v0, uint64_t* add1, uint64_t* add0);
	static void Update(const uint64_t lanes[4], HighwayHashState* state);
	static uint64_t Read64(const uint8_t* src);

	/* Takes a packet of 32 bytes */
	static void HighwayHashUpdatePacket(const uint8_t* packet, HighwayHashState* state);

	static void Rotate32By(uint64_t count, uint64_t lanes[4]);

	/* Adds the final 1..31 bytes, do not use if 0 remain */
	static void HighwayHashUpdateRemainder(const uint8_t* bytes, const size_t size_mod32, HighwayHashState* state);


	static void Permute(const uint64_t v[4], uint64_t* permuted);
	static void PermuteAndUpdate(HighwayHashState* state);
	static void ModularReduction(uint64_t a3_unmasked, uint64_t a2, uint64_t a1, uint64_t a0, uint64_t* m1, uint64_t* m0);


	/* Compute final hash value. Makes state invalid. */
	static uint32_t HighwayHashFinalize32(HighwayHashState* state);
	static uint64_t HighwayHashFinalize64(HighwayHashState* state);
	static void HighwayHashFinalize128(HighwayHashState* state, uint64_t hash[2]);
	static void HighwayHashFinalize256(HighwayHashState* state, uint64_t hash[4]);

	static void ProcessAll(const uint8_t* data, size_t size, const uint64_t key[4], HighwayHashState* state);

	/* Allocates new state for a new streaming hash computation */
	static void HighwayHashCatStart(const uint64_t key[4], HighwayHashCat* state);

	static void HighwayHashCatAppend(const uint8_t* bytes, size_t num, HighwayHashCat* state);

	/* Computes final hash value */
	static uint32_t HighwayHashCatFinish32(const HighwayHashCat* state);
	static uint64_t HighwayHashCatFinish64(const HighwayHashCat* state);
	static void HighwayHashCatFinish128(const HighwayHashCat* state, uint64_t hash[2]);
	static void HighwayHashCatFinish256(const HighwayHashCat* state, uint64_t hash[4]);
};

