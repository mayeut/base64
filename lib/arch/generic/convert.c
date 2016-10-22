static inline uint8_t
convert (const uint16_t in)
{
	unsigned value = in;
#if 0
	unsigned mask = (unsigned)((0 - (int)(value >> 8)) >> (sizeof(int) * 8U - 1U));

	value |= mask;
	return (uint8_t)value;
#else
	return (value > 255U) ? 255U : value;
#endif
}
