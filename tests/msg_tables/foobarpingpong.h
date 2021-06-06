
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
