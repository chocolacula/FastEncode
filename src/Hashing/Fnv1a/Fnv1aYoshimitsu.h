#pragma once

#include <stdint.h>
#include <stdlib.h>

class Fnv1aYoshimitsu
{
private:
	static const uint32_t PRIME = 709607;

public:
	struct State
	{
	private:
		const uint8_t* _start;

	public:
		const uint8_t* Bytes;
		uint32_t A = 2166136261;
		uint32_t B = 2166136261;
		uint32_t C = 2166136261;
		uint32_t D = 2166136261;

		State(const uint8_t* bytes)
		{
			_start = Bytes = bytes;
		}

		bool IsDirty()
		{
			return _start != Bytes;
		}
	};


	static uint32_t Calculate(const uint8_t* bytes, uint32_t size);
	static uint32_t CalculateTriad(const uint8_t* bytes, uint32_t size);
	
	static inline void Append32(State& state);
	static inline void Append24(State& state);
	static inline void Append16(State& state);
	static inline void Append8(State& state);
	static inline void Append4(State& state);
	static inline void Append2(State& state);
	static inline void Append1(State& state);

	static inline uint32_t End(State& state, uint32_t size);
	static inline uint32_t EndTriad(State& state, uint32_t size);
	static inline uint32_t EndTail(State& state, uint32_t size);

	
};