#include "Random.h"

std::random_device Random::_rd;
std::mt19937 Random::_mt(Random::_rd());

const char16_t* Random::NUMBERS = u"0123456789";
const char16_t* Random::SYMBOLS = u"!?@#$%^&*() _+-=[]{},;.:/|\\<>~`\"'";
const char16_t* Random::ENGLISH = u"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char16_t* Random::CHINESE = u"電買車紅無東馬風時鳥語頭魚園長島愛紙書見假佛德拜黑冰兔妒每壤步巢惠鞋莓圓聽實證龍賣龜藝戰繩關鐵圖團轉廣惡豐腦雜壓雞價樂氣廳發勞劍歲權燒贊兩譯觀營處齒驛櫻產藥讀畫顏聲學體點麥蟲舊會萬盜寶國醫雙晝觸來黃區";
const char16_t* Random::CYRILLIC = u"абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";

void Random::RandomiseArray(char16_t* array, size_t length, CharSet chars)
{
	std::u16string source;

	if ((chars & CharSet::Numbers) == CharSet::Numbers)
		source += NUMBERS;
	if ((chars & CharSet::Symbols) == CharSet::Symbols)
		source += SYMBOLS;
	if ((chars & CharSet::English) == CharSet::English)
		source += ENGLISH;
	if ((chars & CharSet::Chinese) == CharSet::Chinese)
		source += CHINESE;
	if ((chars & CharSet::Cyrillic) == CharSet::Cyrillic)
		source += CYRILLIC;

	std::uniform_int_distribution<uint32_t> dist(0, (uint32_t)source.size() - 1);

	for (auto i = 0; i < length; i++)
	{
		array[i] = source[dist(_mt)];
	}
}

void Random::RandomiseArray(uint8_t* array, size_t length)
{
	static const std::uniform_int_distribution<uint32_t> dist(0, 256);

	for (auto i = 0; i < length; i++)
	{
		array[i] = dist(_mt);
	}
}

RandomGenerator Random::GetGenerator(uint32_t from, uint32_t to)
{
	return RandomGenerator(_mt, from, to);
}