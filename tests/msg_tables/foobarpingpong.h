
START_MSG(foo)
FIELD(uint8_t, a)
FIELD(uint8_t, b)
END_MSG(foo)

START_MSG(bar)
FIELD(uint8_t, c)
FIELD(uint8_t, d)
END_MSG(bar)

START_MSG(ping)
FIELD(uint8_t, val)
END_MSG(ping)

START_MSG(pong)
FIELD(uint8_t, val)
END_MSG(pong)


START_MSG(boop)
END_MSG(boop)

START_MSG(bop)
END_MSG(bop)




START_MSG(big)
FIELD(char, c)
FIELD(int8_t, d8)
FIELD(int16_t, d16)
FIELD(int32_t, d32)
FIELD(int64_t, d64)
FIELD(uint8_t, u8)
FIELD(uint16_t, u16)
FIELD(uint32_t, u32)
FIELD(uint64_t, u64)
END_MSG(big)




