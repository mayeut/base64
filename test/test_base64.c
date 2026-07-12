#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/libbase64.h"
#include "codec_supported.h"
#include "moby_dick.h"

static char out[2000];
static size_t outlen;

static uint8_t const ignoreAllChars[] = {
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,       44,  45,  46,      /* 43: '+', 47: '/' */
	 /* 48: '0' -> 57: '9' */                          58,  59,  60,  61,  62,  63,
	 64, /* 65: 'A' -> 90: 'Z' */
								91,  92,  93,  94,  95,
	 96, /* 97: 'a' -> 122: 'z' */
							       123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
};

static bool
assert_enc (int flags, const char *src, const char *dst)
{
	size_t srclen = strlen(src);
	size_t dstlen = strlen(dst);

	base64_encode(src, srclen, out, &outlen, flags);

	if (outlen != dstlen) {
		printf("FAIL: encoding of '%s': length expected %lu, got %lu\n", src,
			(unsigned long)dstlen,
			(unsigned long)outlen
		);
		return true;
	}
	if (strncmp(dst, out, outlen) != 0) {
		out[outlen] = '\0';
		printf("FAIL: encoding of '%s': expected output '%s', got '%s'\n", src, dst, out);
		return true;
	}
	return false;
}

static bool
assert_dec (int flags, const char *src, size_t srclen, const char *dst, const char* ignorechars, size_t ignorecharslen)
{
	if (srclen == 0) {
		srclen = strlen(src);
	}
	size_t dstlen = strlen(dst);

	if (!base64_decode(src, srclen, out, &outlen, flags, ignorechars, ignorecharslen)) {
		printf("FAIL: decoding of '%s': decoding error\n", src);
		return true;
	}
	if (outlen != dstlen) {
		printf("FAIL: decoding of '%s': "
			"length expected %lu, got %lu\n", src,
			(unsigned long)dstlen,
			(unsigned long)outlen
		);
		return true;
	}
	if (strncmp(dst, out, outlen) != 0) {
		out[outlen] = '\0';
		printf("FAIL: decoding of '%s': expected output '%s', got '%s'\n", src, dst, out);
		return true;
	}
	return false;
}

static bool
assert_dec_full (int expected, int flags, const char *src, size_t srclen, const char *dst, const char* ignorechars, size_t ignorecharslen)
{
	if (srclen == 0) {
		srclen = strlen(src);
	}
	size_t dstlen = strlen(dst);

	int ret = base64_decode(src, srclen, out, &outlen, flags, ignorechars, ignorecharslen);

	if (expected == BASE64_DECODE_SUCCESS)
	{
		if (ret <= 0) {
			printf("FAIL: decoding of '%s': decoding error\n", src);
			return true;
		}
		if (outlen != dstlen) {
			printf("FAIL: decoding of '%s': "
			       "length expected %lu, got %lu\n", src,
			       (unsigned long)dstlen,
			       (unsigned long)outlen
			       );
			return true;
		}
		if (strncmp(dst, out, outlen) != 0) {
			out[outlen] = '\0';
			printf("FAIL: decoding of '%s': expected output '%s', got '%s'\n", src, dst, out);
			return true;
		}
		for (size_t bs = 1; bs <= srclen; ++bs) {
			struct base64_state state;
			char const* tmpsrc = src;
			size_t tmpsrclen = srclen;
			char *tmpout = out;
			size_t tmpoutlen;
			outlen = 0;

			base64_stream_decode_init(&state, flags, ignorechars, ignorecharslen);
			for (size_t b = 0; b < ((srclen + (bs - 1)) / bs); ++b, tmpsrc += bs, tmpsrclen -= bs) {
				size_t tmpbs = (tmpsrclen > bs) ? bs : tmpsrclen;
				ret = base64_stream_decode(&state, tmpsrc, tmpbs, tmpout, &tmpoutlen);
				if (ret <= 0) {
					printf("FAIL: decoding of '%s': decoding by %lu error\n", src, (unsigned long)bs);
					return true;
				}
				tmpout += tmpoutlen;
				outlen += tmpoutlen;
			}
			ret = base64_stream_decode_final(&state);
			if ((ret <= 0) || (state.error != BASE64_DECODE_SUCCESS)) {
				printf("FAIL: decoding of '%s': decoding by %lu error\n", src, (unsigned long)bs);
				return true;
			}
			if (outlen != dstlen) {
				printf("FAIL: decoding of '%s': "
				       "length expected %lu, got %lu\n", src,
				       (unsigned long)dstlen,
				       (unsigned long)outlen
				       );
				return true;
			}
			if (strncmp(dst, out, outlen) != 0) {
				out[outlen] = '\0';
				printf("FAIL: decoding of '%s': expected output '%s', got '%s'\n", src, dst, out);
				return true;
			}
		}
	}
	else {
		if (ret > 0) {
			printf("FAIL: decoding of '%s': decoding succeeded\n", src);
			return true;
		}
		for (size_t bs = 1; bs <= srclen; ++bs) {
			struct base64_state state;
			char const* tmpsrc = src;
			size_t tmpsrclen = srclen;

			base64_stream_decode_init(&state, flags, ignorechars, ignorecharslen);
			for (size_t b = 0; b < ((srclen + (bs - 1)) / bs); ++b, tmpsrc += bs, tmpsrclen -= bs) {
				size_t tmpbs = (tmpsrclen > bs) ? bs : tmpsrclen;
				ret = base64_stream_decode(&state, tmpsrc, tmpbs, out, &outlen);
				if ((ret <= 0) && (state.error != expected)) {
					printf("FAIL: decoding of '%s': bad decoding error %d instead of %d\n", src, state.error, expected);
					return true;
				}
			}
			ret = base64_stream_decode_final(&state);
			if ((ret > 0) || (state.error != expected)) {
				printf("FAIL: decoding of '%s': bad decoding error %d instead of %d\n", src, state.error, expected);
				return true;
			}
		}
	}
	return false;
}

static int
assert_roundtrip (int flags, const char *src)
{
	char tmp[1500];
	size_t tmplen;
	size_t srclen = strlen(src);

	// Encode the input into global buffer:
	base64_encode(src, srclen, out, &outlen, flags);

	// Decode the global buffer into local temp buffer:
	if (!base64_decode(out, outlen, tmp, &tmplen, flags, NULL, 0)) {
		printf("FAIL: decoding of '%s': decoding error\n", out);
		return true;
	}

	// Check that 'src' is identical to 'tmp':
	if (srclen != tmplen) {
		printf("FAIL: roundtrip of '%s': "
			"length expected %lu, got %lu\n", src,
			(unsigned long)srclen,
			(unsigned long)tmplen
		);
		return true;
	}
	if (strncmp(src, tmp, tmplen) != 0) {
		tmp[tmplen] = '\0';
		printf("FAIL: roundtrip of '%s': got '%s'\n", src, tmp);
		return true;
	}

	return false;
}

static int
test_char_table (int flags, bool use_malloc)
{
	bool fail = false;
	char chr[256];
	char enc[400], dec[400];
	size_t enclen, declen;

	// Fill array with all characters 0..255:
	for (int i = 0; i < 256; i++)
		chr[i] = (unsigned char)i;

	// Loop, using each char as a starting position to increase test coverage:
	for (int i = 0; i < 256; i++) {

		size_t chrlen = 256 - i;
		char* src = &chr[i];
		if (use_malloc) {
			src = malloc(chrlen); /* malloc/copy this so valgrind can find out-of-bound access */
			if (src == NULL) {
				printf(
					"FAIL: encoding @ %d: allocation of %lu bytes failed\n",
					i, (unsigned long)chrlen
				);
				fail = true;
				continue;
			}
			memcpy(src, &chr[i], chrlen);
		}

		base64_encode(src, chrlen, enc, &enclen, flags);
		if (use_malloc) {
			free(src);
		}

		if (!base64_decode(enc, enclen, dec, &declen, flags, NULL, 0)) {
			printf("FAIL: decoding @ %d: decoding error\n", i);
			fail = true;
			continue;
		}
		if (declen != chrlen) {
			printf("FAIL: roundtrip @ %d: "
				"length expected %lu, got %lu\n", i,
				(unsigned long)chrlen,
				(unsigned long)declen
			);
			fail = true;
			continue;
		}
		if (strncmp(&chr[i], dec, declen) != 0) {
			printf("FAIL: roundtrip @ %d: decoded output not same as input\n", i);
			fail = true;
		}
	}

	return fail;
}

static int
test_streaming (int flags)
{
	bool fail = false;
	char chr[256];
	char ref[400], enc[400];
	size_t reflen;

	// Fill array with all characters 0..255:
	for (int i = 0; i < 256; i++)
		chr[i] = (unsigned char)i;

	// Create reference base64 encoding:
	base64_encode(chr, 256, ref, &reflen, BASE64_FORCE_PLAIN);

	// Encode the table with various block sizes and compare to reference:
	for (size_t bs = 1; bs < 255; bs++)
	{
		size_t inpos   = 0;
		size_t partlen = 0;
		size_t enclen  = 0;
		struct base64_state state;

		base64_stream_encode_init(&state, flags);
		memset(enc, 0, 400);
		for (;;) {
			base64_stream_encode(&state, &chr[inpos], (inpos + bs > 256) ? 256 - inpos : bs, &enc[enclen], &partlen);
			enclen += partlen;
			if (inpos + bs > 256) {
				break;
			}
			inpos += bs;
		}
		base64_stream_encode_final(&state, &enc[enclen], &partlen);
		enclen += partlen;

		if (enclen != reflen) {
			printf("FAIL: stream encoding gave incorrect size: "
				"%lu instead of %lu\n",
				(unsigned long)enclen,
				(unsigned long)reflen
			);
			fail = true;
		}
		if (strncmp(ref, enc, reflen) != 0) {
			printf("FAIL: stream encoding with blocksize %lu failed\n",
				(unsigned long)bs
			);
			fail = true;
		}
	}

	// Decode the reference encoding with various block sizes and
	// compare to input char table:
	for (size_t bs = 1; bs < 255; bs++)
	{
		size_t inpos   = 0;
		size_t partlen = 0;
		size_t enclen  = 0;
		struct base64_state state;

		base64_stream_decode_init(&state, flags, NULL, 0);
		memset(enc, 0, 400);
		for (size_t b = 0; b < ((reflen + (bs - 1)) / bs); ++b, inpos += bs) {
			size_t tmpbs = ((reflen - inpos) > bs) ? bs : reflen - inpos;
			if (base64_stream_decode(&state, &ref[inpos], tmpbs, &enc[enclen], &partlen) <=0) {
				printf(
				       "FAIL: stream decoding with blocksize %lu failed at block start %lu\n",
				       (unsigned long)bs,
				       (unsigned long)inpos
				);
				fail |= true;
				break;
			}
			enclen += partlen;
		}
		if (base64_stream_decode_final(&state) <= 0) {
			printf(
				"FAIL: final stream decoding with blocksize %lu failed\n",
				(unsigned long)bs			);
			fail |= true;
		}
		if (enclen != 256) {
			printf("FAIL: stream decoding gave incorrect size: "
				"%lu instead of 255\n",
				(unsigned long)enclen
			);
			fail = true;
		}
		if (strncmp(chr, enc, 256) != 0) {
			printf("FAIL: stream decoding with blocksize %lu failed\n",
				(unsigned long)bs
			);
			fail = true;
		}
	}

	return fail;
}

static int
test_invalid_dec_input (int flags)
{
	// Subset of invalid characters to cover all ranges
	static const char invalid_set[] = { '\0', -1, '!', '-', ';', '_', '|' };
	static const char* invalid_strings[] = {
		"Zm9vYg=",
		"Zm9vYg",
		"Zm9vY",
		"Zm9vYmF=Zm9v"
	};

	bool fail = false;
	char chr[256];
	char enc[400], dec[400];
	size_t enclen, declen;

	// Fill array with all characters 0..255:
	for (int i = 0; i < 256; i++)
		chr[i] = (unsigned char)i;

	// Create reference base64 encoding:
	base64_encode(chr, 256, enc, &enclen, BASE64_FORCE_PLAIN);

	// Test invalid strings returns error.
	for (size_t i = 0U; i < sizeof(invalid_strings) / sizeof(invalid_strings[0]); ++i) {
		if (base64_decode(invalid_strings[i], strlen(invalid_strings[i]), dec, &declen, flags, NULL, 0)) {
			printf("FAIL: decoding invalid input \"%s\": no decoding error\n", invalid_strings[i]);
			fail = true;
		}
	}

	// Loop, corrupting each char to increase test coverage:
	for (size_t c = 0U; c < sizeof(invalid_set); ++c) {
		for (size_t i = 0U; i < enclen; i++) {
			char backup = enc[i];

			enc[i] = invalid_set[c];

			if (base64_decode(enc, enclen, dec, &declen, flags, NULL, 0)) {
				printf("FAIL: decoding invalid input @ %d: no decoding error\n", (int)i);
				fail = true;
				enc[i] = backup;
				continue;
			}
			enc[i] = backup;
		}
	}

	// Loop, corrupting two chars to increase test coverage:
	for (size_t c = 0U; c < sizeof(invalid_set); ++c) {
		for (size_t i = 0U; i < enclen - 2U; i++) {
			char backup  = enc[i+0];
			char backup2 = enc[i+2];

			enc[i+0] = invalid_set[c];
			enc[i+2] = invalid_set[c];

			if (base64_decode(enc, enclen, dec, &declen, flags, NULL, 0)) {
				printf("FAIL: decoding invalid input @ %d: no decoding error\n", (int)i);
				fail = true;
				enc[i+0] = backup;
				enc[i+2] = backup2;
				continue;
			}
			enc[i+0] = backup;
			enc[i+2] = backup2;
		}
	}

	return fail;
}

static int
test_ignorechars(int flags)
{
	bool fail = false;

	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "ab==:", 0, "i", ":", 1);
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "YW\nJj", 0, "abc", "\n", 1);
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "{YWJj", 0, "abc", "{}", 2);
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "Y}WJj", 0, "abc", "{}", 2);
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "YW{Jj", 0, "abc", "{}", 2);
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "YWJ}j", 0, "abc", "{}", 2);
	// base64 alphabet ignored in ignorechars
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "YWJj", 0, "abc", "Y", 1);
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "YW{Jj", 0, "abc", "Y{", 2);
	// non ASCII
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "YW\x80Jj", 0, "abc", "\x80", 1);
	// NUL character
	fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, "\0ab==", 5, "i", "@\0", 2);

	return fail;
}

static int
test_nonascii(int flags)
{
	bool fail = false;

	// non ASCII
	fail |= assert_dec_full(BASE64_DECODE_ERROR_NON_ASCII, flags | BASE64_REJECT_NON_ASCII, "\x80YWJj", 0, "abc", "\x80", 1);
	fail |= assert_dec_full(BASE64_DECODE_ERROR_NON_ASCII, flags | BASE64_REJECT_NON_ASCII, "Y\x80WJj", 0, "abc", "\x80", 1);
	fail |= assert_dec_full(BASE64_DECODE_ERROR_NON_ASCII, flags | BASE64_REJECT_NON_ASCII, "YW\x80Jj", 0, "abc", "\x80", 1);
	fail |= assert_dec_full(BASE64_DECODE_ERROR_NON_ASCII, flags | BASE64_REJECT_NON_ASCII, "YWJ\x80j", 0, "abc", "\x80", 1);

	return fail;
}

static int
test_padded_option(int flags)
{
	bool fail = false;

	// Test vectors:
	struct {
		const char *in;
		const char *out;
		int padded_error_code;
	} vec[] = {
		{"", "", BASE64_DECODE_SUCCESS},
		{"YQ==", "a", BASE64_DECODE_SUCCESS},
		{"YQ", "a", BASE64_DECODE_ERROR_INCORRECT_PADDING},
		{"YQ=", "a", BASE64_DECODE_ERROR_INCORRECT_PADDING},
		{"YWI=", "ab", BASE64_DECODE_SUCCESS},
		{"YWI", "ab", BASE64_DECODE_ERROR_INCORRECT_PADDING},
		{"YWJj", "abc", BASE64_DECODE_SUCCESS},
		{"=YWJj", "abc", BASE64_DECODE_SUCCESS},
		{"Y=WJj", "abc", BASE64_DECODE_SUCCESS},
		{"YW=Jj", "abc", BASE64_DECODE_SUCCESS},
		{"YWJ=j", "abc", BASE64_DECODE_SUCCESS},
	};

	for (size_t i = 0; i < sizeof(vec) / sizeof(vec[0]); i++) {
		/* test with padded = false & validation */
		if (strchr(vec[i].in, '=') != NULL) {
			fail |= assert_dec_full(BASE64_DECODE_ERROR_PADDING_NOT_ALLOWED, flags | BASE64_NO_PADDING, vec[i].in, 0, vec[i].out, NULL, 0);
		}
		else {
			fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags | BASE64_NO_PADDING, vec[i].in, 0, vec[i].out, NULL, 0);
			fail |= assert_enc(flags | BASE64_NO_PADDING, vec[i].out, vec[i].in);
		}
		/* test with padded = true & all ignore chars */
		fail |= assert_dec_full(vec[i].padded_error_code, flags, vec[i].in, 0, vec[i].out, (char const*)ignoreAllChars, sizeof(ignoreAllChars));
		fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags | BASE64_NO_PADDING, vec[i].in, 0, vec[i].out, "=", 1);
	}

	return fail;
}

static int
test_partially_invalid(int flags)
{
	bool fail = false;

	// Test vectors:
	struct {
		const char *in;
		const char *out;
		const char *ignorechars;
		size_t ignorecharslen;
		int error;
		int ignorechars_error;
	} vec[] = {
		// excess padding
		{"ab===", "i", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"ab====", "i", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abc==", "i\xb7", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abc===", "i\xb7", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abc====", "i\xb7", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abc=====", "i\xb7", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd=", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd==", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd===", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd====", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd=====", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd=efgh", "i\xb7\x1dy\xf8!", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd==efgh", "i\xb7\x1dy\xf8!", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd===efgh", "i\xb7\x1dy\xf8!", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd====efgh", "i\xb7\x1dy\xf8!", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"abcd=====efgh", "i\xb7\x1dy\xf8!", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		{"YWJj=", "abc", "=", 1, BASE64_DECODE_ERROR_EXCESS_PADDING, BASE64_DECODE_SUCCESS},
		// leading padding
		{"=", "", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"==", "", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"===", "", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"====", "", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"=====", "", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"=abcd", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"==abcd", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"===abcd", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"====abcd", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"=====abcd", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		{"[==", "", "[=", 2, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"=YWJj", "abc", "=", 1, BASE64_DECODE_ERROR_LEADING_PADDING, BASE64_DECODE_SUCCESS},
		// invalid length
		{"a=b==", "i", "=", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"a=bc=", "i\xb7", "=", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"a=bc==", "i\xb7", "=", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"a=bcd", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"a=bcd=", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		// discontinuous padding
		{"ab=c=", "i\xb7", "=", 1, BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING, BASE64_DECODE_SUCCESS},
		{"ab=cd", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING, BASE64_DECODE_SUCCESS},
		{"ab=cd==", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING, BASE64_DECODE_SUCCESS},
		{"Y=WJj", "abc", "=", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"Y==WJj", "abc", "=", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"YW=Jj", "abc", "=", 1, BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING, BASE64_DECODE_SUCCESS},
		// excess data
		{"ab==cd", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_EXCESS_DATA, BASE64_DECODE_SUCCESS},
		{"abc=d", "i\xb7\x1d", "=", 1, BASE64_DECODE_ERROR_EXCESS_DATA, BASE64_DECODE_SUCCESS},
		// invalid data
		{"ab:(){:|:&};:==", "i", ":;(){}|&", 8, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"\nab==", "i", "\n", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"ab==\n", "i", "\n", 1, BASE64_DECODE_ERROR_EXCESS_DATA, BASE64_DECODE_SUCCESS},
		{"a\nb==", "i", "\n", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"a\1b==", "i", "\1", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"a\1b==", "i", "@\1", 2, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"\1ab==", "i", "@\1", 2, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"ab:==", "i", ":", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_SUCCESS},
		{"ab=:=", "i", ":", 1, BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING, BASE64_DECODE_SUCCESS},
		{"ab==:", "i", ":", 1, BASE64_DECODE_ERROR_EXCESS_DATA, BASE64_DECODE_SUCCESS},
		{"abc=:", "i\xb7", ":", 1, BASE64_DECODE_ERROR_EXCESS_DATA, BASE64_DECODE_SUCCESS},
		{"@ab==", "i", ":", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_ERROR_INVALID_DATA},
		{"ab@==", "i", ":", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_ERROR_INVALID_DATA},
		{"ab=@=", "i", ":", 1, BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING, BASE64_DECODE_ERROR_DISCONTINUOUS_PADDING},
		{"abc@=", "i\xb7", ":", 1, BASE64_DECODE_ERROR_INVALID_DATA, BASE64_DECODE_ERROR_INVALID_DATA},
	};

	for (size_t i = 0; i < sizeof(vec) / sizeof(vec[0]); i++) {
		fail |= assert_dec_full(vec[i].error, flags, vec[i].in, 0, vec[i].out, NULL, 0);
		fail |= assert_dec_full(vec[i].ignorechars_error, flags, vec[i].in, 0, vec[i].out, vec[i].ignorechars, strlen(vec[i].ignorechars));
		fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, vec[i].in, 0, vec[i].out, (char const*)ignoreAllChars, sizeof(ignoreAllChars));
	}
	return fail;
}

static int
test_canonical (int flags)
{
	bool fail = false;

	// Test vectors:
	struct {
		const char *in;
		const char *out;
		int error;
	} vec[] = {
		{"", "", BASE64_DECODE_SUCCESS},
		{"Zg==", "f", BASE64_DECODE_SUCCESS},
		{"Zm8=", "fo", BASE64_DECODE_SUCCESS},
		{"Zm9v", "foo", BASE64_DECODE_SUCCESS},
		{"Zh==", "f", BASE64_DECODE_ERROR_PADDING_BITS_NOT_ALLOWED},
		{"Zm9=", "fo", BASE64_DECODE_ERROR_PADDING_BITS_NOT_ALLOWED},
	};

	for (size_t i = 0; i < sizeof(vec) / sizeof(vec[0]); i++) {
		fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, vec[i].in, 0, vec[i].out, NULL, 0);
		fail |= assert_dec_full(vec[i].error, flags | BASE64_CANONICAL, vec[i].in, 0, vec[i].out, NULL, 0);
	}
	return fail;
}

static int
test_one_codec (size_t codec_index)
{
	bool fail = false;
	const char *codec = codecs[codec_index];

	printf("Codec %s:\n", codec);

	// Skip if this codec is not supported:
	int flags = codec_supported(codec_index);
	if (flags == 0) {
		puts("  skipping");
		return false;
	}

	// Test vectors:
	struct {
		const char *in;
		const char *out;
		const char *linefeed;
	} vec[] = {

		// These are the test vectors from RFC4648:
		{ "",		"",         "\n"},
		{ "f",		"Zg==",     "\nZ\ng\n=\n=\n"},
		{ "fo",		"Zm8=",     "Zm\n8="},
		{ "foo",	"Zm9v",     "Z\nm9v"},
		{ "foob",	"Zm9vYg==", "\nZm9vYg=="},
		{ "fooba",	"Zm9vYmE=", "Zm9vYmE=\n"},
		{ "foobar",	"Zm9vYmFy", "Zm9v\nYmFy"},

		// The first paragraph from Moby Dick,
		// to test the SIMD codecs with larger blocksize:
		{ moby_dick_plain, moby_dick_base64, NULL },
	};

	for (size_t i = 0; i < sizeof(vec) / sizeof(vec[0]); i++) {

		// Encode plain string, check against output:
		fail |= assert_enc(flags, vec[i].in, vec[i].out);

		// Decode the output string, check if we get the input:
		fail |= assert_dec(flags, vec[i].out, 0, vec[i].in, NULL, 0);
		if (vec[i].linefeed != NULL) {
			fail |= assert_dec_full(BASE64_DECODE_SUCCESS, flags, vec[i].linefeed, 0, vec[i].in, "\n", 1);
		}
		// Do a roundtrip on the inputs and the outputs:
		fail |= assert_roundtrip(flags, vec[i].in);
		fail |= assert_roundtrip(flags, vec[i].out);
	}
	fail |= test_ignorechars(flags);
	fail |= test_nonascii(flags);
	fail |= test_partially_invalid(flags);
	fail |= test_padded_option(flags);
	fail |= test_canonical(flags);

	fail |= test_char_table(flags, false); /* test with unaligned input buffer */
	fail |= test_char_table(flags, true); /* test for out-of-bound input read */
	fail |= test_streaming(flags);
	fail |= test_invalid_dec_input(flags);

	if (!fail)
		puts("  all tests passed.");

	return fail;
}

int
main ()
{
	bool fail = false;

	// Loop over all codecs:
	for (size_t i = 0; codecs[i]; i++) {
		// Test this codec, merge the results:
		fail |= test_one_codec(i);
	}

	return (fail) ? 1 : 0;
}
