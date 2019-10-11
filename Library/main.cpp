#include <cstdint>

extern "C"
{
	void __declspec(dllexport) toBase64(const uint8_t* input, uint32_t inputSize, uint16_t* output, uint32_t outputSize)
	{
		//defaultConverter.toBase64(input, inputSize, output, outputSize);
	}

	void __declspec(dllexport) fromBase64(const uint16_t* input, uint32_t inputSize, uint8_t* output, uint32_t outputSize, uint32_t addition)
	{
		//defaultConverter.fromBase64(input, inputSize, output, outputSize, addition);
	}
}