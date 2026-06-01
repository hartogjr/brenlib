/** @author   Bren de Hartog <bren@dehartog.name>
 * @copyright Copyright (c) 2026, Bren de Hartog. All rights reserved.
 * @license   This project is licensed under 3-clause BSD license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <cstdlib>
#include <regex>
#include <stdexcept>
#include <string>
#include <fmt/format.h>
#include <bren/Logger.hpp>
#include <bren/NaiveTime.hpp>

namespace Bren {

	NaiveTime::NaiveTime()
	: hour_(0xFF), min_(0xFF)
	{ }

	uint8_t NaiveTime::hour() const
	{
		FCET(hour_ <= 23, std::logic_error, "Internal hour not initialized yet");
		return hour_;
	}

	void NaiveTime::hour(const uint8_t hour_i)
	{
		FCET(hour_i <= 23, std::invalid_argument, "Given hour {:d} should be <= 23", hour_i);
		hour_ = hour_i;
	}

	uint8_t NaiveTime::minute() const
	{
		FCET(min_ <= 59, std::logic_error, "Internal minute not initialized yet");
		return min_;
	}

	void NaiveTime::minute(const uint8_t minute_i)
	{
		FCET(minute_i <= 59, std::invalid_argument, "Given minute {:d} should be <= 59", minute_i);
		min_ = minute_i;
	}

	int16_t NaiveTime::operator-(const NaiveTime & rhs) const
	{
		if (!valid_()) throw std::logic_error("Lefthand-side object is not valid");
		if (!rhs.valid_()) throw std::invalid_argument("Righthand-side object is not valid");

		return (hour_ * 60 + min_) - (rhs.hour_ * 60 + rhs.min_);
	}

	NaiveTime & NaiveTime::operator+=(const uint16_t min_i)
	{
		FCET(valid_(), std::logic_error, "Internal time not (fully) initialized");
		if (min_i == 0) return *this;

		std::div_t qr = std::div(hour_ * 60 + min_ + min_i, 60);
		if (qr.quot > 23) throw std::overflow_error("Minutes addition would exceed midnight");
		hour_ = qr.quot;
		min_ = qr.rem;
		return *this;
	}

	bool NaiveTime::operator<(const NaiveTime & rhs) const
	{
		if (!valid_()) throw std::logic_error("Lefthand-side object is not valid");
		if (!rhs.valid_()) throw std::invalid_argument("Righthand-side object is not valid");
		return hour_ < rhs.hour_ || (hour_ == rhs.hour_ && min_ < rhs.min_);
	}

	bool NaiveTime::operator<=(const NaiveTime & rhs) const
	{
		if (!valid_()) throw std::logic_error("Lefthand-side object is not valid");
		if (!rhs.valid_()) throw std::invalid_argument("Righthand-side object is not valid");
		return hour_ < rhs.hour_ || (hour_ == rhs.hour_ && min_ <= rhs.min_);
	}

	NaiveTime & NaiveTime::operator=(const std::string & time_i)
	{
		FCET(time_i.size() >= 3 && time_i.size() <= 5, std::invalid_argument,
			"String length of given time '{:s}' should be 3, 4 or 5 characters, not {:d}",
			time_i, time_i.size());

		/// Time Regular Expression
		std::regex tre("^([012]?[0-9]):([0-5]?[0-9])$", std::regex_constants::extended);
		std::smatch matches;
		std::regex_search(time_i, matches, tre);
		FCET(matches.size() == 3, std::invalid_argument,
			"Found {:d} regex matches for time instead of 3", matches.size());

		int hour = std::stoi(matches[1]);
		int min = std::stoi(matches[2]);

		FCET(hour <= 23 && min <= 59, std::invalid_argument,
			"Unable to parse '{:s}' as a valid 24-hour time", time_i);
		hour_ = hour;
		min_ = min;

		return *this;
	}

	bool NaiveTime::operator>(const NaiveTime & rhs) const
	{
		if (!valid_()) throw std::logic_error("Lefthand-side object is not valid");
		if (!rhs.valid_()) throw std::invalid_argument("Righthand-side object is not valid");
		return hour_ > rhs.hour_ || (hour_ == rhs.hour_ && min_ > rhs.min_);
	}

	bool NaiveTime::operator>=(const NaiveTime & rhs) const
	{
		if (!valid_()) throw std::logic_error("Lefthand-side object is not valid");
		if (!rhs.valid_()) throw std::invalid_argument("Righthand-side object is not valid");
		return hour_ > rhs.hour_ || (hour_ == rhs.hour_ && min_ >= rhs.min_);
	}

	bool NaiveTime::operator==(const NaiveTime & rhs) const
	{
		if (!valid_()) throw std::logic_error("Lefthand-side object is not valid");
		if (!rhs.valid_()) throw std::invalid_argument("Righthand-side object is not valid");
		return hour_ == rhs.hour_ && min_ == rhs.min_;
	}

	NaiveTime::operator std::string() const
	{
		FCET(valid_(), std::logic_error, "Internal time not (fully) initialized");
		return fmt::format("{:02d}:{:02d}", hour_, min_);
	}

} // Bren namespace
