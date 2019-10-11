#pragma once

#include <stdint.h>

class XorHash
{
public:
	static inline uint32_t Calculate(const uint8_t* bytes, uint32_t size)
	{
		uint64_t hash = 0;

		/*while (size >= 4)
		{
			hash ^=  *(uint32_t*)bytes;
			
			// mov eax, DWORD PTR hash$[rsp]
			// mov rcx, QWORD PTR bytes$[rsp]
			// xor eax, DWORD PTR[rcx]
			// mov DWORD PTR hash$[rsp], eax
			
			bytes += 4;
			size -= 4;
		}

		while (size >= 32)
		{
			hash ^= *(uint32_t*)(bytes)      ^ *(uint32_t*)(bytes + 4)  ^ *(uint32_t*)(bytes + 8)  ^ *(uint32_t*)(bytes + 12)
				  ^ *(uint32_t*)(bytes + 16) ^ *(uint32_t*)(bytes + 20) ^ *(uint32_t*)(bytes + 24) ^ *(uint32_t*)(bytes + 28);
			
			// it is twice faster than variant above because of decreased number of jmp and mov instructions
			// mov rax, QWORD PTR bytes$[rsp]
			// mov rcx, QWORD PTR bytes$[rsp]
			// mov ecx, DWORD PTR[rcx + 4]
			// mov eax, DWORD PTR[rax]
			// xor eax, ecx
			// mov rcx, QWORD PTR bytes$[rsp]
			// xor eax, DWORD PTR[rcx + 8]
			// mov rcx, QWORD PTR bytes$[rsp]
			// xor eax, DWORD PTR[rcx + 12]
			// mov rcx, QWORD PTR bytes$[rsp]
			// xor eax, DWORD PTR[rcx + 16]
			// mov rcx, QWORD PTR bytes$[rsp]
			// xor eax, DWORD PTR[rcx + 20]
			// mov rcx, QWORD PTR bytes$[rsp]
			// xor eax, DWORD PTR[rcx + 24]
			// mov rcx, QWORD PTR bytes$[rsp]
			// xor eax, DWORD PTR[rcx + 28]
			// mov ecx, DWORD PTR hash$[rsp]
			// xor ecx, eax
			// mov eax, ecx
			// mov DWORD PTR hash$[rsp], eax
						
			bytes += 32;
			size -= 32;
		}*/

		while (size >= 64)
		{
			hash ^= *(uint64_t*)bytes ^ *(uint64_t*)(bytes + 8) 
				 ^ *(uint64_t*)(bytes + 16) ^ *(uint64_t*)(bytes + 24)
				 ^ *(uint64_t*)(bytes + 32) ^ *(uint64_t*)(bytes + 40)
				 ^ *(uint64_t*)(bytes + 48) ^ *(uint64_t*)(bytes + 56);

			bytes += 64;
			size -= 64;
		}

		while (size >= 32)
		{
			hash ^= *(uint64_t*)bytes ^ *(uint64_t*)(bytes + 8)
				 ^ *(uint64_t*)(bytes + 16) ^ *(uint64_t*)(bytes + 24);

			bytes += 32;
			size -= 32;
		}

		if (size >= 16)
		{
			hash ^= *(uint64_t*)bytes ^ *(uint64_t*)(bytes + 8);

			bytes += 16;
			size -= 16;
		}

		if (size >= 8)
		{
			hash ^= *(uint64_t*)bytes;

			bytes += 16;
			size -= 16;
		}

		if (size >= 4)
		{
			hash ^= *(uint32_t*)bytes;

			bytes += 4;
			size -= 4;
		}

		if (size >= 2)
		{
			hash ^= *(uint16_t*)bytes;

			bytes += 2;
			size -= 2;
		}

		if (size == 1)
			hash ^= *bytes;

		hash ^= 4910230523947010759;

		return (uint32_t)hash ^ *(((uint32_t*)&hash) + 1);
	}
};