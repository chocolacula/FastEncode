#pragma once

#include <immintrin.h>

#define CMPGT(s,n)		_mm256_cmpgt_epi8((s), _mm256_set1_epi8(n))
#define CMPEQ(s,n)		_mm256_cmpeq_epi8((s), _mm256_set1_epi8(n))
#define REPLACE(s,n)	_mm256_and_si256((s),  _mm256_set1_epi8(n))
#define RANGE(s,a,b)	_mm256_andnot_si256(CMPGT((s), (b)), CMPGT((s), (a) - 1))

static inline __m256i EncryptReshuffle(const __m256i input)
{
	// translation from SSSE3 into AVX2 of procedure
	// This one works with shifted (4 bytes) input in order to
	// be able to work efficiently in the 2 128-bit lanes

	// input, bytes MSB to LSB:
	// 0 0 0 0 x w v u t s r q p o n m
	// l k j i h g f e d c b a 0 0 0 0

	auto shufled = _mm256_permutevar8x32_epi32(input, _mm256_setr_epi32(0, 0, 1, 2, 3, 4, 5, 6));

	shufled = _mm256_shuffle_epi8(shufled, _mm256_set_epi8(
		10, 11, 9, 10,
		7, 8, 6, 7,
		4, 5, 3, 4,
		1, 2, 0, 1,

		14, 15, 13, 14,
		11, 12, 10, 11,
		8, 9, 7, 8,
		5, 6, 4, 5));
	// in, bytes MSB to LSB:
	// w x v w
	// t u s t
	// q r p q
	// n o m n
	// k l j k
	// h i g h
	// e f d e
	// b c a b

	const __m256i t0 = _mm256_and_si256(shufled, _mm256_set1_epi32(0x0fc0fc00));
	// bits, upper case are most significant bits, lower case are least significant bits.
	// 0000wwww XX000000 VVVVVV00 00000000
	// 0000tttt UU000000 SSSSSS00 00000000
	// 0000qqqq RR000000 PPPPPP00 00000000
	// 0000nnnn OO000000 MMMMMM00 00000000
	// 0000kkkk LL000000 JJJJJJ00 00000000
	// 0000hhhh II000000 GGGGGG00 00000000
	// 0000eeee FF000000 DDDDDD00 00000000
	// 0000bbbb CC000000 AAAAAA00 00000000

	const __m256i t1 = _mm256_mulhi_epu16(t0, _mm256_set1_epi32(0x04000040));
	// 00000000 00wwwwXX 00000000 00VVVVVV
	// 00000000 00ttttUU 00000000 00SSSSSS
	// 00000000 00qqqqRR 00000000 00PPPPPP
	// 00000000 00nnnnOO 00000000 00MMMMMM
	// 00000000 00kkkkLL 00000000 00JJJJJJ
	// 00000000 00hhhhII 00000000 00GGGGGG
	// 00000000 00eeeeFF 00000000 00DDDDDD
	// 00000000 00bbbbCC 00000000 00AAAAAA

	const __m256i t2 = _mm256_and_si256(shufled, _mm256_set1_epi32(0x003f03f0));
	// 00000000 00xxxxxx 000000vv WWWW0000
	// 00000000 00uuuuuu 000000ss TTTT0000
	// 00000000 00rrrrrr 000000pp QQQQ0000
	// 00000000 00oooooo 000000mm NNNN0000
	// 00000000 00llllll 000000jj KKKK0000
	// 00000000 00iiiiii 000000gg HHHH0000
	// 00000000 00ffffff 000000dd EEEE0000
	// 00000000 00cccccc 000000aa BBBB0000

	const __m256i t3 = _mm256_mullo_epi16(t2, _mm256_set1_epi32(0x01000010));
	// 00xxxxxx 00000000 00vvWWWW 00000000
	// 00uuuuuu 00000000 00ssTTTT 00000000
	// 00rrrrrr 00000000 00ppQQQQ 00000000
	// 00oooooo 00000000 00mmNNNN 00000000
	// 00llllll 00000000 00jjKKKK 00000000
	// 00iiiiii 00000000 00ggHHHH 00000000
	// 00ffffff 00000000 00ddEEEE 00000000
	// 00cccccc 00000000 00aaBBBB 00000000

	return _mm256_or_si256(t1, t3);
	// 00xxxxxx 00wwwwXX 00vvWWWW 00VVVVVV
	// 00uuuuuu 00ttttUU 00ssTTTT 00SSSSSS
	// 00rrrrrr 00qqqqRR 00ppQQQQ 00PPPPPP
	// 00oooooo 00nnnnOO 00mmNNNN 00MMMMMM
	// 00llllll 00kkkkLL 00jjKKKK 00JJJJJJ
	// 00iiiiii 00hhhhII 00ggHHHH 00GGGGGG
	// 00ffffff 00eeeeFF 00ddEEEE 00DDDDDD
	// 00cccccc 00bbbbCC 00aaBBBB 00AAAAAA
}

static inline __m256i EncryptTranslate(const __m256i input)
{
	// LUT contains Absolute offset for all ranges:
	static const __m256i lut = _mm256_setr_epi8(65, 71, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -19, -16, 0, 0,
										        65, 71, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -19, -16, 0, 0);
	// Translate values 0..63 to the Base64 alphabet. There are five sets:
	// #  From      To         Abs    Index  Characters
	// 0  [0..25]   [65..90]   +65        0  ABCDEFGHIJKLMNOPQRSTUVWXYZ
	// 1  [26..51]  [97..122]  +71        1  abcdefghijklmnopqrstuvwxyz
	// 2  [52..61]  [48..57]    -4  [2..11]  0123456789
	// 3  [62]      [43]       -19       12  +
	// 4  [63]      [47]       -16       13  /

	// Create LUT indexes from input:
	// the index for range #0 is right, others are 1 less than expected:
	__m256i indexes = _mm256_subs_epu8(input, _mm256_set1_epi8(51));

	// mask is 255 (255) for range #[1..4] and 0x00 for range #0:
	__m256i mask = CMPGT(input, 25);

	// substract 255, so add 1 to indexes for range #[1..4], All indexes are now correct:
	indexes = _mm256_sub_epi8(indexes, mask);

	// Add offsets to input values:
	__m256i out = _mm256_add_epi8(input, _mm256_shuffle_epi8(lut, indexes));

	return out;
}

static inline __m256i DecryptReshuffle(__m256i input)
{
	// in, lower lane, bits, upper case are most significant bits, lower case are least significant bits:
	// 00llllll 00kkkkLL 00jjKKKK 00JJJJJJ
	// 00iiiiii 00hhhhII 00ggHHHH 00GGGGGG
	// 00ffffff 00eeeeFF 00ddEEEE 00DDDDDD
	// 00cccccc 00bbbbCC 00aaBBBB 00AAAAAA

	const __m256i merge_ab_and_bc = _mm256_maddubs_epi16(input, _mm256_set1_epi32(0x01400140));
	// 0000kkkk LLllllll 0000JJJJ JJjjKKKK
	// 0000hhhh IIiiiiii 0000GGGG GGggHHHH
	// 0000eeee FFffffff 0000DDDD DDddEEEE
	// 0000bbbb CCcccccc 0000AAAA AAaaBBBB

	__m256i out = _mm256_madd_epi16(merge_ab_and_bc, _mm256_set1_epi32(0x00011000));
	// 00000000 JJJJJJjj KKKKkkkk LLllllll
	// 00000000 GGGGGGgg HHHHhhhh IIiiiiii
	// 00000000 DDDDDDdd EEEEeeee FFffffff
	// 00000000 AAAAAAaa BBBBbbbb CCcccccc

	// Pack bytes together in each lane:
	out = _mm256_shuffle_epi8(out, _mm256_setr_epi8(
		2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, -1, -1, -1, -1,
		2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, -1, -1, -1, -1));
	// 00000000 00000000 00000000 00000000
	// LLllllll KKKKkkkk JJJJJJjj IIiiiiii
	// HHHHhhhh GGGGGGgg FFffffff EEEEeeee
	// DDDDDDdd CCcccccc BBBBbbbb AAAAAAaa

	// Pack lanes
	return _mm256_permutevar8x32_epi32(out, _mm256_setr_epi32(0, 1, 2, 4, 5, 6, -1, -1));
}

static inline __m256i DecryptTranslate(const __m256i input)
{
	const __m256i lut_lo = _mm256_setr_epi8(
		0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
		0x11, 0x11, 0x13, 0x1A, 0x1B, 0x1B, 0x1B, 0x1A,
		0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
		0x11, 0x11, 0x13, 0x1A, 0x1B, 0x1B, 0x1B, 0x1A);

	const __m256i lut_hi = _mm256_setr_epi8(
		0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08,
		0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
		0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08,
		0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10);

	const __m256i lut_roll = _mm256_setr_epi8(
		0, 16, 19, 4, -65, -65, -71, -71,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 16, 19, 4, -65, -65, -71, -71,
		0, 0, 0, 0, 0, 0, 0, 0);

	const __m256i mask_2F = _mm256_set1_epi8(0x2f);

	// lookup
	const __m256i hi_nibbles = _mm256_and_si256(_mm256_srli_epi32(input, 4), mask_2F);
	const __m256i lo_nibbles = _mm256_and_si256(input, mask_2F);
	const __m256i hi = _mm256_shuffle_epi8(lut_hi, hi_nibbles);
	const __m256i lo = _mm256_shuffle_epi8(lut_lo, lo_nibbles);
	const __m256i eq_2F = _mm256_cmpeq_epi8(input, mask_2F);
	const __m256i roll = _mm256_shuffle_epi8(lut_roll, _mm256_add_epi8(eq_2F, hi_nibbles));

//	if (!_mm256_testz_si256(lo, hi)) {
//		break;
//	}

	// Now simply add the delta values to the input:
	return _mm256_add_epi8(input, roll);
}