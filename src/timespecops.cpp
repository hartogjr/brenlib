/* Copyright (c) 2025 Bren de Hartog <bren@dehartog.name>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE. */

#include <iostream>
#include <iomanip>
#include <string.h>
#include <bren/timespecops.hpp>

namespace std {

	bool operator<(const struct timespec & lhs, const struct timespec & rhs)
	{
		return (
				   lhs.tv_sec < rhs.tv_sec || (
					   lhs.tv_sec == rhs.tv_sec &&
					   lhs.tv_nsec < rhs.tv_nsec
				   )
			   );

	}

	bool operator<=(const struct timespec & lhs, const struct timespec & rhs)
	{
		return lhs == rhs || lhs < rhs;
	}

	bool operator>(const struct timespec & lhs, const struct timespec & rhs)
	{
		return ~(lhs <= rhs);
	}

	bool operator>=(const struct timespec & lhs, const struct timespec & rhs)
	{
		return ~(lhs < rhs);
	}

	bool operator==(const struct timespec & lhs, const struct timespec & rhs)
	{
		return lhs.tv_sec == rhs.tv_sec && lhs.tv_nsec == rhs.tv_nsec;
	}

	bool operator!=(const struct timespec & lhs, const struct timespec & rhs)
	{
		return ~(lhs == rhs);
	}

	void operator-=(struct timespec & lhs, const struct timespec & rhs)
	{
		lhs.tv_sec -= rhs.tv_sec;

		if (lhs.tv_nsec < rhs.tv_nsec) {
			lhs.tv_nsec = lhs.tv_nsec + 1000000000 - rhs.tv_nsec;
			lhs.tv_sec--;
		}
		else lhs.tv_nsec -= rhs.tv_nsec;
	}

	struct timespec operator+(const struct timespec & lhs, const struct timespec & rhs)
	{
		struct timespec rv;

		memcpy(&rv, &lhs, sizeof(struct timespec));
		rv.tv_sec += rhs.tv_sec;
		rv.tv_nsec += rhs.tv_nsec;

		if (rv.tv_nsec >= 1000000000) {
			rv.tv_nsec -= 1000000000;
			rv.tv_sec++;
		}

		return rv;
	}

	std::ostream & operator<<(std::ostream & os_i, const struct timespec & ts)
	{
		os_i << std::dec << ts.tv_sec << '.' << std::setw(9) << std::setfill('0') << ts.tv_nsec;
		return os_i;
	}

} // std namespace
