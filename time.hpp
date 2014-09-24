#ifndef MSL_2011_TIME_HPP
#define MSL_2011_TIME_HPP

#include <cstdint>

namespace msl
{
	std::int64_t nanis();
	std::int64_t micris();
	std::int64_t millis();

	void delay_ns(const std::int64_t nanoseconds);
	void delay_us(const std::int64_t microseconds);
	void delay_ms(const std::int64_t milliseconds);
}

#endif