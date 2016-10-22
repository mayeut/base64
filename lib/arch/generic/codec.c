#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "../../../include/libbase64.h"
#include "../../codecs.h"

#include "../generic/convert.c"

BASE64_ENC_FUNCTION(plain)
{
	#include "enc_head.c"
#if BASE64_WORDSIZE == 32
	#include "32/enc_loop.c"
#elif BASE64_WORDSIZE == 64
	#include "64/enc_loop.c"
#endif
	#include "enc_tail.c"
}

#define STRING_TYPE uint8_t
#define CHAR_CONVERT(x) (x)
BASE64_DEC_FUNCTION(plain)
{
	#include "dec_head.c"
#if BASE64_WORDSIZE == 32
	#include "32/dec_loop.c"
#elif BASE64_WORDSIZE == 64
	#include "64/dec_loop.c"
#endif
	#include "dec_tail.c"
}
#undef CHAR_CONVERT
#undef STRING_TYPE

#define STRING_TYPE uint16_t
#define CHAR_CONVERT(x) convert(x)
BASE64_DEC16_FUNCTION(plain)
{
#include "dec_head.c"
#include "dec_tail.c"
}
#undef CHAR_CONVERT
#undef STRING_TYPE

BASE64_CVT_FUNCTION(plain)
{
	#include "convert_loop.c"
}
