#pragma once

#include "Base64_sse_codec.h"
#include "../../Base64_default.h"

class Base64_sse
{
public:
	static void ToBase64(const uint8_t* input, uint32_t inputSize, char16_t* output);
	static void FromBase64(const char16_t* input, uint32_t inputSize, uint8_t* output, uint32_t addition);
};