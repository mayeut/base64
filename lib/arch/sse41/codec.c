#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "../../../include/libbase64.h"
#include "../../codecs.h"

#ifdef __SSE4_1__
#include <smmintrin.h>

#include "../sse2/compare_macros.h"

#include "../ssse3/_mm_bswap_epi32.c"
#include "../ssse3/dec_reshuffle.c"
#include "../ssse3/enc_translate.c"
#include "enc_reshuffle.c"

#include "../generic/convert.c"

#endif	// __SSE4_1__

BASE64_ENC_FUNCTION(sse41)
{
#ifdef __SSE4_1__
	#include "../generic/enc_head.c"
	#include "../ssse3/enc_loop.c"
	#include "../generic/enc_tail.c"
#else
	BASE64_ENC_STUB
#endif
}

#define STRING_TYPE uint8_t
#define CHAR_CONVERT(x) (x)
#define LOAD_STRING(c) LOAD_STRING8(c)
BASE64_DEC_FUNCTION(sse41)
{
#ifdef __SSE4_1__
	#include "../generic/dec_head.c"
	#include "../ssse3/dec_loop.c"
	#include "../generic/dec_tail.c"
#else
	BASE64_DEC_STUB
#endif
}
#undef LOAD_STRING
#undef CHAR_CONVERT
#undef STRING_TYPE

#define STRING_TYPE uint16_t
#define CHAR_CONVERT(x) convert(x)
#define LOAD_STRING(c) LOAD_STRING16(c)
BASE64_DEC16_FUNCTION(sse41)
{
#ifdef __SSE4_1__
#include "../generic/dec_head.c"
#include "../ssse3/dec_loop.c"
#include "../generic/dec_tail.c"
#else
	BASE64_DEC_STUB
#endif
}
#undef LOAD_STRING
#undef CHAR_CONVERT
#undef STRING_TYPE

BASE64_CVT_FUNCTION(sse41)
{
#ifdef __SSE4_1__
	#include "../sse2/convert_loop.c"
	#include "../generic/convert_loop.c"
#else
	BASE64_CVT_STUB
#endif
}
