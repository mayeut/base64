while (len >= 64U) {
	__m128i src0 = _mm_loadu_si128((const __m128i*)(src +  0));
	__m128i src1 = _mm_loadu_si128((const __m128i*)(src +  8));
	__m128i src2 = _mm_loadu_si128((const __m128i*)(src + 16));
	__m128i src3 = _mm_loadu_si128((const __m128i*)(src + 24));
	__m128i src4 = _mm_loadu_si128((const __m128i*)(src + 32));
	__m128i src5 = _mm_loadu_si128((const __m128i*)(src + 40));
	__m128i src6 = _mm_loadu_si128((const __m128i*)(src + 48));
	__m128i src7 = _mm_loadu_si128((const __m128i*)(src + 56));


	__m128i dst0 = _mm_packus_epi16(src0, src1);
	__m128i dst1 = _mm_packus_epi16(src2, src3);
	__m128i dst2 = _mm_packus_epi16(src4, src5);
	__m128i dst3 = _mm_packus_epi16(src6, src7);

	_mm_storeu_si128((__m128i*)(dst +  0), dst0);
	_mm_storeu_si128((__m128i*)(dst + 16), dst1);
	_mm_storeu_si128((__m128i*)(dst + 32), dst2);
	_mm_storeu_si128((__m128i*)(dst + 48), dst3);

	len-= 64U;
	src += 64U;
	dst += 64U;
}
if (len & 32U) {
	__m128i src0 = _mm_loadu_si128((const __m128i*)(src +  0));
	__m128i src1 = _mm_loadu_si128((const __m128i*)(src +  8));
	__m128i src2 = _mm_loadu_si128((const __m128i*)(src + 16));
	__m128i src3 = _mm_loadu_si128((const __m128i*)(src + 24));

	__m128i dst0 = _mm_packus_epi16(src0, src1);
	__m128i dst1 = _mm_packus_epi16(src2, src3);

	_mm_storeu_si128((__m128i*)(dst +  0), dst0);
	_mm_storeu_si128((__m128i*)(dst + 16), dst1);

	len-= 32U;
	src += 32U;
	dst += 32U;
}
if (len & 16U) {
	__m128i src0 = _mm_loadu_si128((const __m128i*)(src +  0));
	__m128i src1 = _mm_loadu_si128((const __m128i*)(src +  8));

	__m128i dst0 = _mm_packus_epi16(src0, src1);

	_mm_storeu_si128((__m128i*)(dst +  0), dst0);

	len-= 16U;
	src += 16U;
	dst += 16U;
}
