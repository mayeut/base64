		if (slen-- == 0) {
			ret = 1;
			break;
		}
		c = *s++;
		if ((q = base64_table_dec_8bit[c]) >= 254) {
			if (check_ignore(c, state)) {
				continue;
			}
			if (q == 254) {
				if (state->flags & BASE64_NO_PADDING) {
					state->error = BASE64_DECODE_ERROR_PADDING_NOT_ALLOWED;
				}
				else if (((state->flags & BASE64_STARTED) == 0) && ((o - (uint8_t*)out) == 0)) {
					state->error = BASE64_DECODE_ERROR_LEADING_PADDING;
				}
				else {
					state->error = BASE64_DECODE_ERROR_EXCESS_PADDING;
				}
			}
			else {
				state->error = ((c < 128) || ((state->flags & BASE64_REJECT_NON_ASCII) == 0)) ? BASE64_DECODE_ERROR_INVALID_DATA : BASE64_DECODE_ERROR_NON_ASCII;
			}
			break;
		}
		st.carry = q << 2;
		st.bytes++;

		// Deliberate fallthrough:
		BASE64_FALLTHROUGH

	case 1:
CASE_1:
		if (slen-- == 0) {
			ret = 1;
			break;
		}
		c = *s++;
		if ((q = base64_table_dec_8bit[c]) >= 254) {
			if (check_ignore(c, state)) {
				goto CASE_1;
			}
			if ((q == 254) && (state->flags & BASE64_NO_PADDING)) {
				state->error = BASE64_DECODE_ERROR_PADDING_NOT_ALLOWED;
			}
			else {
				state->error = ((c < 128) || ((state->flags & BASE64_REJECT_NON_ASCII) == 0)) ? BASE64_DECODE_ERROR_INVALID_DATA : BASE64_DECODE_ERROR_NON_ASCII;
			}
			break;
		}
		*o++ = st.carry | (q >> 4);
		st.carry = q << 4;
		st.bytes++;
		olen++;

		// Deliberate fallthrough:
		BASE64_FALLTHROUGH

	case 2:
CASE_2:
		if (slen-- == 0) {
			ret = 1;
			break;
		}
		c = *s++;
		if ((q = base64_table_dec_8bit[c]) >= 254) {
			// When q == 254, the input char is '='.
			// Check if next byte is also '=':
			if (q == 254) {
				if ((state->flags & BASE64_NO_PADDING) && !check_ignore('=', state)) {
					state->error = BASE64_DECODE_ERROR_PADDING_NOT_ALLOWED;
					break;
				}
				if (next_valid_padding(&s, &slen, state) == 254) {
					/* if the next valid byte is '=' => end or skip depending on '=' being in ignorechars */
					if (check_excess_data(&s, &slen, state)) {
						if (check_ignore('=', state)) {
							/* restart at excess data */
							goto CASE_2;
						}
						state->error = (*s == '=') ? BASE64_DECODE_ERROR_EXCESS_PADDING : BASE64_DECODE_ERROR_EXCESS_DATA;
						break;
					}
					st.bytes = 0;
					st.eof = BASE64_EOF2;
					ret = 1;
					break;
				}
				else if (slen == 0) {
					// Almost EOF
					st.eof = BASE64_AEOF;
					ret = 1;
					break;
				}
				else if (!check_ignore('=', state)) {
					state->error = BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING;
					break;
				}
			}
			if (check_ignore(c, state)) {
				goto CASE_2;
			}
			// If we get here, there was an error:
			state->error = ((c < 128) || ((state->flags & BASE64_REJECT_NON_ASCII) == 0)) ? BASE64_DECODE_ERROR_INVALID_DATA : BASE64_DECODE_ERROR_NON_ASCII;
			break;
		}
		*o++ = st.carry | (q >> 2);
		st.carry = q << 6;
		st.bytes++;
		olen++;

		// Deliberate fallthrough:
		BASE64_FALLTHROUGH

	case 3:
CASE_3:
		if (slen-- == 0) {
			ret = 1;
			break;
		}
		c = *s++;
		if ((q = base64_table_dec_8bit[c]) >= 254) {
			if (q == 254) {
				if ((state->flags & BASE64_NO_PADDING) && !check_ignore('=', state)) {
					state->error = BASE64_DECODE_ERROR_PADDING_NOT_ALLOWED;
					break;
				}
				if (check_excess_data(&s, &slen, state)) {
					if (check_ignore('=', state)) {
						goto CASE_3;
					}
					state->error = (*s == '=') ? BASE64_DECODE_ERROR_EXCESS_PADDING : BASE64_DECODE_ERROR_EXCESS_DATA;
					break;
				}
				st.bytes = 0;
				st.eof = BASE64_EOF;
				ret = 1;
				break;
			}
			if (check_ignore(c, state)) {
				goto CASE_3;
			}
			// If we get here, there was an error:
			state->error = ((c < 128) || ((state->flags & BASE64_REJECT_NON_ASCII) == 0)) ? BASE64_DECODE_ERROR_INVALID_DATA : BASE64_DECODE_ERROR_NON_ASCII;
			break;
		}
		*o++ = st.carry | q;
		st.carry = 0;
		st.bytes = 0;
		olen++;
	}
}

state->eof = st.eof;
state->bytes = st.bytes;
state->carry = st.carry;
state->flags |= BASE64_STARTED;
*outlen = olen;
return ret;
