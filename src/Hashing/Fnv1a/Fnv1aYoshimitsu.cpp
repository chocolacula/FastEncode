#pragma once

#include "Fnv1aYoshimitsu.h"

uint32_t Fnv1aYoshimitsu::Calculate(const uint8_t* bytes, uint32_t size)
{
	State state(bytes);

	for (; size >= 32; size -= 32)
	{
		Append32(state);
	}

	return End(state, size);
}

uint32_t Fnv1aYoshimitsu::CalculateTriad(const uint8_t* bytes, uint32_t size)
{
	State state(bytes);

	for (; size >= 24; size -= 24)
	{
		Append24(state);
	}

	return EndTriad(state, size);
}

void Fnv1aYoshimitsu::Append32(State& state)
{
	state.A = (state.A ^ (_rotl(*(uint32_t*)(state.Bytes + 0), 5)  ^ *(uint32_t*)(state.Bytes + 4)))  * PRIME;
	state.B = (state.B ^ (_rotl(*(uint32_t*)(state.Bytes + 8), 5)  ^ *(uint32_t*)(state.Bytes + 12))) * PRIME;
	state.C = (state.C ^ (_rotl(*(uint32_t*)(state.Bytes + 16), 5) ^ *(uint32_t*)(state.Bytes + 20))) * PRIME;
	state.D = (state.D ^ (_rotl(*(uint32_t*)(state.Bytes + 24), 5) ^ *(uint32_t*)(state.Bytes + 28))) * PRIME;

	state.Bytes += 32;
}

void Fnv1aYoshimitsu::Append24(State& state)
{
	state.A = (state.A ^ (_rotl(*(uint32_t*)(state.Bytes + 0), 5)  ^ *(uint32_t*)(state.Bytes + 4)))  * PRIME;
	state.B = (state.B ^ (_rotl(*(uint32_t*)(state.Bytes + 8), 5)  ^ *(uint32_t*)(state.Bytes + 12))) * PRIME;
	state.C = (state.C ^ (_rotl(*(uint32_t*)(state.Bytes + 16), 5) ^ *(uint32_t*)(state.Bytes + 20))) * PRIME;

	state.Bytes += 24;
}

void Fnv1aYoshimitsu::Append16(State& state)
{
	state.A = (state.A ^ (_rotl(*(uint32_t*)(state.Bytes + 0), 5) ^ *(uint32_t*)(state.Bytes + 4))) * PRIME;
	state.B = (state.B ^ (_rotl(*(uint32_t*)(state.Bytes + 8), 5) ^ *(uint32_t*)(state.Bytes + 12))) * PRIME;

	state.Bytes += 16;
}

void Fnv1aYoshimitsu::Append8(State& state)
{
	state.A = (state.A ^ *(uint32_t*)(state.Bytes + 0)) * PRIME;
	state.B = (state.B ^ *(uint32_t*)(state.Bytes + 4)) * PRIME;

	state.Bytes += 8;
}

void Fnv1aYoshimitsu::Append4(State& state)
{
	state.A = (state.A ^ *(uint16_t*)(state.Bytes + 0)) * PRIME;
	state.B = (state.B ^ *(uint16_t*)(state.Bytes + 2)) * PRIME;

	state.Bytes += 4;
}

void Fnv1aYoshimitsu::Append2(State& state)
{
	state.A = (state.A ^ *(uint16_t*)state.Bytes) * PRIME;

	state.Bytes += 2;
}

void Fnv1aYoshimitsu::Append1(State& state)
{
	state.A = (state.A ^ *state.Bytes) * PRIME;

	state.Bytes++;
}

uint32_t Fnv1aYoshimitsu::End(State& state, uint32_t size)
{
	if (state.IsDirty())
	{
		state.A = (state.A ^ _rotl(state.C, 5)) * PRIME;
		state.B = (state.B ^ _rotl(state.D, 5)) * PRIME;
	}

	return EndTail(state, size);
}

uint32_t Fnv1aYoshimitsu::EndTriad(State& state, uint32_t size)
{
	if (state.IsDirty())
	{
		state.A = (state.A ^ _rotl(state.C, 5)) * PRIME;
	}

	return EndTail(state, size);
}

uint32_t Fnv1aYoshimitsu::EndTail(State& state, uint32_t size)
{
	// 1111=15; 10111=23
	if (size & 16)
		Append16(state);

	// Cases: 0,1,2,3,4,5,6,7,...,15
	if (size & 8)
		Append8(state);

	// Cases: 0,1,2,3,4,5,6,7
	if (size & 4)
		Append4(state);

	if (size & 2)
		Append2(state);

	if (size & 1)
		Append1(state);

	state.A = (state.A ^ _rotl(state.B, 5)) * PRIME;
	return  state.A ^ (state.A >> 16);
}