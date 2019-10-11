#pragma once

#include <arm_neon.h>
#include "../../Base64_default.h"

#define CMPGT(s,n)	vcgtq_u8((s), vdupq_n_u8(n))

// Encoding
// Use a 64-byte lookup to do the encoding.
// Reuse existing base64_table_enc table.
static const uint8_t* SYMBOL_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const __n128 tbl_enc = vld1q_u8(SYMBOL_TABLE);


// Decoding
// The input consists of five valid character sets in the Base64 alphabet,
// which we need to map back to the 6-bit values they represent.
// There are three ranges, two singles, and then there's the rest.
//
//   #  From       To        LUT  Characters
//   1  [0..42]    [255]      #1  invalid input
//   2  [43]       [62]       #1  +
//   3  [44..46]   [255]      #1  invalid input
//   4  [47]       [63]       #1  /
//   5  [48..57]   [52..61]   #1  0..9
//   6  [58..63]   [255]      #1  invalid input
//   7  [64]       [255]      #2  invalid input
//   8  [65..90]   [0..25]    #2  A..Z
//   9  [91..96]   [255]      #2 invalid input
//  10  [97..122]  [26..51]   #2  a..z
//  11  [123..126] [255]      #2 invalid input
// (12) Everything else => invalid input

// First LUT will use VTBL instruction (out of range indices are set to 0 in destination).
static const uint8_t base64_dec_lut1[] =
{
	255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U,
	255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U,
	255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U, 255U,  62U, 255U, 255U, 255U,  63U,
	 52U,  53U,  54U,  55U,  56U,  57U,  58U,  59U,  60U,  61U, 255U, 255U, 255U, 255U, 255U, 255U
};
// Second LUT will use VTBX instruction (out of range indices will be unchanged in destination).
// Input [64..126] will be mapped to index [1..63] in this LUT. Index 0 means that value comes from LUT #1.
static const uint8_t base64_dec_lut2[] =
{
	  0U, 255U,   0U,   1U,   2U,   3U,   4U,   5U,   6U,   7U,   8U,   9U,  10U,  11U,  12U,  13U,
	 14U,  15U,  16U,  17U,  18U,  19U,  20U,  21U,  22U,  23U,  24U,  25U, 255U, 255U, 255U, 255U,
	255U, 255U,  26U,  27U,  28U,  29U,  30U,  31U,  32U,  33U,  34U,  35U,  36U,  37U,  38U,  39U,
	 40U,  41U,  42U,  43U,  44U,  45U,  46U,  47U,  48U,  49U,  50U,  51U, 255U, 255U, 255U, 255U
};

// All input values in range for the first look-up will be 0U in the second look-up result.
// All input values out of range for the first look-up will be 0U in the first look-up result.
// Thus, the two results can be ORed without conflicts.
// Invalid characters that are in the valid range for either look-up will be set to 255U in the combined result.
// Other invalid characters will just be passed through with the second look-up result (using VTBX instruction).
// Since the second LUT is 64 bytes, those passed through values are guaranteed to have a value greater than 63U.
// Therefore, valid characters will be mapped to the valid [0..63] range and all invalid characters will be mapped
// to values greater than 63.

static inline uint8x16x4_t EncryptReshuffle(const uint8x16x3_t input)
{
	uint8x16x4_t output;

	// Divide bits of three input bytes over four output bytes:
	output.val[0] = vshrq_n_u8(input.val[0], 2);
	output.val[1] = vshrq_n_u8(input.val[1], 4) | vshlq_n_u8(input.val[0], 4);
	output.val[2] = vshrq_n_u8(input.val[2], 6) | vshlq_n_u8(input.val[1], 2);
	output.val[3] = input.val[2];

	// Clear top two bits:
	output.val[0] &= vdupq_n_u8(0x3F);
	output.val[1] &= vdupq_n_u8(0x3F);
	output.val[2] &= vdupq_n_u8(0x3F);
	output.val[3] &= vdupq_n_u8(0x3F);

	return output;
}

static inline uint8x16x4_t EncryptTranslate(const uint8x16x4_t input)
{
	uint8x16x4_t output;

	// The bits have now been shifted to the right locations;
	// translate their values 0..63 to the Base64 alphabet.
	// Use a 64-byte table lookup:
	output.val[0] = vqtbl4q_u8(tbl_enc, input.val[0]);
	output.val[1] = vqtbl4q_u8(tbl_enc, input.val[1]);
	output.val[2] = vqtbl4q_u8(tbl_enc, input.val[2]);
	output.val[3] = vqtbl4q_u8(tbl_enc, input.val[3]);

	return output;
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