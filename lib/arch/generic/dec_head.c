int ret = 0;
const uint8_t *s = (const uint8_t *) src;
uint8_t *o = (uint8_t *) out;
uint8_t c;
uint8_t q;

// Use local temporaries to avoid cache thrashing:
size_t olen = 0;
size_t slen = srclen;
struct base64_state st;
st.eof = state->eof;
st.bytes = state->bytes;
st.carry = state->carry;

// If we had a previous error, bail out:
if (state->error) {
	*outlen = 0;
	return 0;
}

if (slen == 0) {
	*outlen = 0;
	return 1;
}

// If we previously saw an EOF or an invalid character, bail out:
if (st.eof) {
	if (st.eof == BASE64_AEOF) {
		if (next_valid_padding(&s, &slen, state) == 254) {
			if (!check_excess_data(&s, &slen, state)) {
				/* update state */
				state->bytes = 0;
				state->eof = BASE64_EOF2;
				*outlen = 0;
				return 1;
			}
			if (!check_ignore('=', state)) {
				state->error = (*s == '=') ? BASE64_DECODE_ERROR_EXCESS_PADDING : BASE64_DECODE_ERROR_EXCESS_DATA;
				*outlen = 0;
				return 0;
			}
		}
		else if (slen == 0) {
			*outlen = 0;
			return 1;
		}
		else if (!check_ignore('=', state)) {
			state->error = BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING;
			*outlen = 0;
			return 0;
		}
		/* restart */
		st.eof = 0;
		st.bytes = 2;
	}
	else {
		/* BASE64_EOF or BASE64_EOF2 */
		if (!check_excess_data(&s, &slen, state)) {
			/* no state change */
			*outlen = 0;
			return 1;
		}
		if (!check_ignore('=', state)) {
			state->error = (*s == '=') ? BASE64_DECODE_ERROR_EXCESS_PADDING : BASE64_DECODE_ERROR_EXCESS_DATA;
			*outlen = 0;
			return 0;
		}
		/* restart at excess data */
		st.bytes = (st.eof == BASE64_EOF) ? 3 : 2;
		st.eof = 0;
	}
}

// Turn four 6-bit numbers into three bytes:
// out[0] = 11111122
// out[1] = 22223333
// out[2] = 33444444

// Duff's device again:
switch (st.bytes)
{
	for (;;)
	{
	case 0:
