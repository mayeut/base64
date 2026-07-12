#include "../include/libbase64.h"
#include "tables/tables.h"
#include <stdint.h>

static int
check_ignore(uint8_t c, struct base64_state *state)
{
	if (state->ignorecache[c >> 3] & (1U << (c & 7U))) {
		return 1;
	}
	if ((c < 128) || ((state->flags & BASE64_REJECT_NON_ASCII) == 0)) {
		uint8_t const* ignorechars = (uint8_t const*)state->ignorechars;
		for (size_t i = 0; i < state->ignorecharslen; ++i) {
			if (c == ignorechars[i]) {
				state->ignorecache[c >> 3] |= 1U << (c & 7U);
				return 1;
			}
		}
	}
	return 0;
}

static int
check_excess_data(const uint8_t** pSrc, size_t* pSrclen, struct base64_state *state)
{
    uint8_t const* src = *pSrc;
    size_t srclen = *pSrclen;
    while (srclen && check_ignore(*src, state)) {
	src++;
	srclen--;
    }
    *pSrc = src;
    *pSrclen = srclen;
    return srclen > 0;
}

static int
next_valid_padding(uint8_t const** pSrc, size_t* pSrclen, struct base64_state *state)
{
    int ret = 255;
    uint8_t const* src = *pSrc;
    size_t srclen = *pSrclen;
    while (srclen && (*src != '=') && check_ignore(*src, state)) {
	src++;
	srclen--;
    }
    if (srclen > 0) {
	ret = base64_table_dec_8bit[*src];
	if (ret == 254) {
		/* only consume padding byte here */
		src++;
		srclen--;
	}
    }
    *pSrc = src;
    *pSrclen = srclen;
    return ret;
}
