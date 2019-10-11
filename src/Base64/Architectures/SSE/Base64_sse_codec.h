#pragma once

#include <nmmintrin.h>

#define CMPGT(s,n)		_mm_cmpgt_epi8((s), _mm_set1_epi8(n))
#define CMPEQ(s,n)		_mm_cmpeq_epi8((s), _mm_set1_epi8(n))
#define REPLACE(s,n)	_mm_and_si128((s), _mm_set1_epi8(n))
#define RANGE(s,a,b)	_mm_andnot_si128(CMPGT((s), (b)), CMPGT((s), (a) - 1))

static inline __m128i EncryptReshuffle(const __m128i input)
{
	// input, bytes MSB to LSB:
	// 0 0 0 0 l k j i h g f e d c b a

	auto shuffled = _mm_shuffle_epi8(input, _mm_set_epi8(
		10, 11, 9, 10,
		7, 8, 6, 7,
		4, 5, 3, 4,
		1, 2, 0, 1));

	// in, bytes MSB to LSB:
	// k l j k
	// h i g h
	// e f d e
	// b c a b

	const __m128i t0 = _mm_and_si128(shuffled, _mm_set1_epi32(0x0fc0fc00));
	// bits, upper case are most significant bits, lower case are least significant bits
	// 0000kkkk LL000000 JJJJJJ00 00000000
	// 0000hhhh II000000 GGGGGG00 00000000
	// 0000eeee FF000000 DDDDDD00 00000000
	// 0000bbbb CC000000 AAAAAA00 00000000

	const __m128i t1 = _mm_mulhi_epu16(t0, _mm_set1_epi32(0x04000040));
	// 00000000 00kkkkLL 00000000 00JJJJJJ
	// 00000000 00hhhhII 00000000 00GGGGGG
	// 00000000 00eeeeFF 00000000 00DDDDDD
	// 00000000 00bbbbCC 00000000 00AAAAAA

	const __m128i t2 = _mm_and_si128(shuffled, _mm_set1_epi32(0x003f03f0));
	// 00000000 00llllll 000000jj KKKK0000
	// 00000000 00iiiiii 000000gg HHHH0000
	// 00000000 00ffffff 000000dd EEEE0000
	// 00000000 00cccccc 000000aa BBBB0000

	const __m128i t3 = _mm_mullo_epi16(t2, _mm_set1_epi32(0x01000010));
	// 00llllll 00000000 00jjKKKK 00000000
	// 00iiiiii 00000000 00ggHHHH 00000000
	// 00ffffff 00000000 00ddEEEE 00000000
	// 00cccccc 00000000 00aaBBBB 00000000

	return _mm_or_si128(t1, t3);
	// 00llllll 00kkkkLL 00jjKKKK 00JJJJJJ
	// 00iiiiii 00hhhhII 00ggHHHH 00GGGGGG
	// 00ffffff 00eeeeFF 00ddEEEE 00DDDDDD
	// 00cccccc 00bbbbCC 00aaBBBB 00AAAAAA
}

static inline __m128i EncryptTranslate(const __m128i input)
{
	// LUT contains Absolute offset for all ranges:
	const __m128i lut = _mm_setr_epi8(
		65, 71, -4, -4,
		-4, -4, -4, -4,
		-4, -4, -4, -4,
		-19, -16, 0, 0
	);

	// Translate values 0..63 to the Base64 alphabet. There are five sets:
	// #  From      To         Abs    Index  Characters
	// 0  [0..25]   [65..90]   +65        0  ABCDEFGHIJKLMNOPQRSTUVWXYZ
	// 1  [26..51]  [97..122]  +71        1  abcdefghijklmnopqrstuvwxyz
	// 2  [52..61]  [48..57]    -4  [2..11]  0123456789
	// 3  [62]      [43]       -19       12  +
	// 4  [63]      [47]       -16       13  /

	// Create LUT indices from input:
	// the index for range #0 is right, others are 1 less than expected:
	__m128i indices = _mm_subs_epu8(input, _mm_set1_epi8(51));

	// mask is 0xFF (-1) for range #[1..4] and 0x00 for range #0:
	__m128i mask = CMPGT(input, 25);

	// substract -1, so add 1 to indices for range #[1..4], All indices are now correct:
	indices = _mm_sub_epi8(indices, mask);

	// Add offsets to input values:
	__m128i out = _mm_add_epi8(input, _mm_shuffle_epi8(lut, indices));

	return out;
}

static inline __m128i DecryptReshuffle(__m128i input)
{
	// in, bits, upper case are most significant bits, lower case are least significant bits
	// 00llllll 00kkkkLL 00jjKKKK 00JJJJJJ
	// 00iiiiii 00hhhhII 00ggHHHH 00GGGGGG
	// 00ffffff 00eeeeFF 00ddEEEE 00DDDDDD
	// 00cccccc 00bbbbCC 00aaBBBB 00AAAAAA

	const __m128i merge_ab_and_bc = _mm_maddubs_epi16(input, _mm_set1_epi32(0x01400140));
	// 0000kkkk LLllllll 0000JJJJ JJjjKKKK
	// 0000hhhh IIiiiiii 0000GGGG GGggHHHH
	// 0000eeee FFffffff 0000DDDD DDddEEEE
	// 0000bbbb CCcccccc 0000AAAA AAaaBBBB

	const __m128i out = _mm_madd_epi16(merge_ab_and_bc, _mm_set1_epi32(0x00011000));
	// 00000000 JJJJJJjj KKKKkkkk LLllllll
	// 00000000 GGGGGGgg HHHHhhhh IIiiiiii
	// 00000000 DDDDDDdd EEEEeeee FFffffff
	// 00000000 AAAAAAaa BBBBbbbb CCcccccc

	// Pack bytes together:
	return  _mm_shuffle_epi8(out, _mm_setr_epi8(
		2, 1, 0,
		6, 5, 4,
		10, 9, 8,
		14, 13, 12,
		-1, -1, -1, -1));
	// 00000000 00000000 00000000 00000000
	// LLllllll KKKKkkkk JJJJJJjj IIiiiiii
	// HHHHhhhh GGGGGGgg FFffffff EEEEeeee
	// DDDDDDdd CCcccccc BBBBbbbb AAAAAAaa
}

static inline __m128i DecryptTranslate(const __m128i input)
{
	// The input consists of six character sets in the Base64 alphabet,
	// which we need to map back to the 6-bit values they represent.
	// There are three ranges, two singles, and then there's the rest.
	//
	//  #  From       To        Add  Characters
	//  1  [43]       [62]      +19  +
	//  2  [47]       [63]      +16  /
	//  3  [48..57]   [52..61]   +4  0..9
	//  4  [65..90]   [0..25]   -65  A..Z
	//  5  [97..122]  [26..51]  -71  a..z
	// (6) Everything else => invalid input

	// We will use LUTS for character validation & offset computation
	// Remember that 0x2X and 0x0X are the same index for _mm_shuffle_epi8,
	// this allows to mask with 0x2F instead of 0x0F and thus save one constant declaration (register and/or memory access)

	// For offsets:
	// Perfect hash for lut = ((src>>4)&0x2F)+((src==0x2F)?0xFF:0x00)
	// 0000 = garbage
	// 0001 = /
	// 0010 = +
	// 0011 = 0-9
	// 0100 = A-Z
	// 0101 = A-Z
	// 0110 = a-z
	// 0111 = a-z
	// 1000 >= garbage

	// For validation, here's the table.
	// A character is valid if and only if the AND of the 2 lookups equals 0:

	// hi \ lo              0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111
	//      LUT             0x15 0x11 0x11 0x11 0x11 0x11 0x11 0x11 0x11 0x11 0x13 0x1A 0x1B 0x1B 0x1B 0x1A

	// 0000 0X10 char        NUL  SOH  STX  ETX  EOT  ENQ  ACK  BEL   BS   HT   LF   VT   FF   CR   SO   SI
	//           andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10

	// 0001 0x10 char        DLE  DC1  DC2  DC3  DC4  NAK  SYN  ETB  CAN   EM  SUB  ESC   FS   GS   RS   US
	//           andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10

	// 0010 0x01 char               !    "    #    $    %    &    '    (    )    *    +    ,    -    .    /
	//           andlut     0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x01 0x00 0x01 0x01 0x01 0x00

	// 0011 0x02 char          0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ?
	//           andlut     0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x02 0x02 0x02 0x02 0x02 0x02

	// 0100 0x04 char          @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    0
	//           andlut     0x04 0x00 0x00 0x00 0X00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00

	// 0101 0x08 char          P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _
	//           andlut     0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x08 0x08 0x08 0x08 0x08

	// 0110 0x04 char          `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o
	//           andlut     0x04 0x00 0x00 0x00 0X00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
	// 0111 0X08 char          p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~
	//           andlut     0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x08 0x08 0x08 0x08 0x08

	// 1000 0x10 andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10
	// 1001 0x10 andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10
	// 1010 0x10 andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10
	// 1011 0x10 andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10
	// 1100 0x10 andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10
	// 1101 0x10 andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10
	// 1110 0x10 andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10
	// 1111 0x10 andlut     0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10 0x10

	const __m128i lut_lo = _mm_setr_epi8(
		0x15, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
		0x11, 0x11, 0x13, 0x1A, 0x1B, 0x1B, 0x1B, 0x1A);

	const __m128i lut_hi = _mm_setr_epi8(
		0x10, 0x10, 0x01, 0x02, 0x04, 0x08, 0x04, 0x08,
		0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10);

	const __m128i lut_roll = _mm_setr_epi8(
		0, 16, 19, 4, -65, -65, -71, -71,
		0, 0, 0, 0, 0, 0, 0, 0);

	const __m128i mask_2F = _mm_set1_epi8(0x2f);

	// lookup
	const __m128i hi_nibbles = _mm_and_si128(_mm_srli_epi32(input, 4), mask_2F);
	const __m128i lo_nibbles = _mm_and_si128(input, mask_2F);
	const __m128i hi = _mm_shuffle_epi8(lut_hi, hi_nibbles);
	const __m128i lo = _mm_shuffle_epi8(lut_lo, lo_nibbles);
	const __m128i eq_2F = _mm_cmpeq_epi8(input, mask_2F);
	const __m128i roll = _mm_shuffle_epi8(lut_roll, _mm_add_epi8(eq_2F, hi_nibbles));

	// Check for invalid input: if any "and" values from lo and hi are not zero,
	// fall back on bytewise code to do error checking and reporting:
	//if (_mm_movemask_epi8(CMPGT(_mm_and_si128(lo, hi), 0)) != 0) {
	//	break;
	//}

	// Now simply add the delta values to the input:
	return _mm_add_epi8(input, roll);
}
