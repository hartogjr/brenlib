/** @author   Bren de Hartog <bren@dehartog.name>
 * @copyright Copyright (c) 2025, Bren de Hartog. All rights reserved.
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

#include <cstdint>
#include <ctime>
#include <limits>
#include <stdexcept>
#include <string>
#include <bren/Logger.hpp>
#include <bren/NaiveDate.hpp>

namespace Bren {

	NaiveDate::NaiveDate()
	: day_(0), month_(0), year_(0)
	{ }

	uint8_t NaiveDate::day() const
	{
		FCET(day_, std::logic_error, "Day of month not initialized yet");
		return day_;
	}

	void NaiveDate::day(const uint8_t day_i)
	{
		FCET(day_i >= 1 && day_i <= 31, std::invalid_argument,
			"Given day of month '{:d}' not in the range between 1 and 31", day_i);
		day_ = day_i;
	}

	uint8_t NaiveDate::days(const uint16_t year_i, const uint8_t month_i)
	{
		FCET(month_i > 0 && month_i <= 12, std::invalid_argument,
			"Given month {:d} should be between 1 and 12", month_i);

		switch (month_i) {
			case 2:
				if (leap_(year_i)) return 29;
				else return 28;

			case 4:
			case 6:
			case 5:
			case 9:
			case 11:
				return 30;

			default:
				return 31;
		}
	}

	void NaiveDate::iso8601(const std::string & date_i)
	{
		FCET(date_i.size() == 10, std::invalid_argument,
			"ISO8601 date '{:s}' is not exactly 10 characters long, but {:d}",
			date_i, date_i.size());

		int year, mon, day;

		year = std::stoi(date_i.substr(0, 4));
		FCET(year >= 1900 && year <= std::numeric_limits<uint16_t>::max(), std::invalid_argument,
			"Year not between 1900 and the max for uint16_t ({:d})",
			std::numeric_limits<uint16_t>::max());

		mon = std::stoi(date_i.substr(5, 2));
		FCET(mon >= 1 && mon <= 12, std::invalid_argument, "Month not between 1 and 12");

		day = std::stoi(date_i.substr(8, 2));
		FCET(day >= 1 && day <= 31, std::invalid_argument, "Day of month not between 1 and 31");

		FCET(NaiveDate::valid(year, mon, day), std::invalid_argument,
			"Date '{:s}' is not valid.", date_i);

		year_ = static_cast<uint16_t>(year);
		month_ = static_cast<uint8_t>(mon);
		day_ = static_cast<uint8_t>(day);
	}

	std::string NaiveDate::iso8601() const
	{
		FCET(initialized_(), std::logic_error, "Internal date is not valid");
		return fmt::format("{:04d}-{:02d}-{:02d}", year_ , month_, day_);
	}

	bool NaiveDate::leap_(const uint16_t year_i)
	{
		if (year_i % 400 == 0) return true;
		if (year_i % 100 == 0) return false;
		return (year_i % 4 == 0);
	}

	bool NaiveDate::leap() const
	{
		FCET(year_, std::logic_error, "Internal date is not valid");
		return leap_(year_);
	}

	uint8_t NaiveDate::month() const
	{
		FCET(month_, std::logic_error, "Month not initialized yet");
		return month_;
	}

	void NaiveDate::month(const uint8_t month_i)
	{
		FCET(month_i >= 1 && month_i <= 12, std::invalid_argument,
			"Given month '{:d}' not in the range between 1 and 12", month_i);
		month_ = month_i;
	}

	bool NaiveDate::operator<(const NaiveDate & rhs)
	{
		FCET(valid() && rhs.valid(), std::invalid_argument, "Can't compare invalid date");
		return
			year_ < rhs.year_ ||
			(year_ == rhs.year_ && month_ < rhs.month_) ||
			(year_ == rhs.year_ && month_ == rhs.month_ && day_ < rhs.day_);
	}

	bool NaiveDate::operator<=(const NaiveDate & rhs)
	{
		FCET(valid() && rhs.valid(), std::invalid_argument, "Can't compare invalid date");
		return
			year_ < rhs.year_ ||
			(year_ == rhs.year_ && month_ < rhs.month_) ||
			(year_ == rhs.year_ && month_ == rhs.month_ && day_ <= rhs.day_);
	}

	bool NaiveDate::operator==(const NaiveDate & rhs)
	{
		FCET(valid() && rhs.valid(), std::invalid_argument, "Can't compare invalid date");
		return year_ == rhs.year_ && month_ == rhs.month_ && day_ == rhs.day_;
	}

	bool NaiveDate::operator!=(const NaiveDate & rhs)
	{
		FCET(valid() && rhs.valid(), std::invalid_argument, "Can't compare invalid date");
		return year_ != rhs.year_ || month_ != rhs.month_ || day_ != rhs.day_;
	}

	bool NaiveDate::operator>=(const NaiveDate & rhs)
	{
		FCET(valid() && rhs.valid(), std::invalid_argument, "Can't compare invalid date");
		return
			year_ > rhs.year_ ||
			(year_ == rhs.year_ && month_ > rhs.month_) ||
			(year_ == rhs.year_ && month_ == rhs.month_ && day_ >= rhs.day_);
	}

	bool NaiveDate::operator>(const NaiveDate & rhs)
	{
		FCET(valid() && rhs.valid(), std::invalid_argument, "Can't compare invalid date");
		return
			year_ > rhs.year_ ||
			(year_ == rhs.year_ && month_ > rhs.month_) ||
			(year_ == rhs.year_ && month_ == rhs.month_ && day_ > rhs.day_);
	}

	NaiveDate NaiveDate::today()
	{
		NaiveDate nd;

		std::time_t now = std::time(nullptr);
		std::tm *bdt = std::localtime(&now);
		nd.year(bdt->tm_year + 1900);
		nd.month(bdt->tm_mon + 1);
		nd.day(bdt->tm_mday);
		return nd;
	}

	bool NaiveDate::valid(const uint16_t year_i, const uint8_t month_i, const uint8_t day_i)
	{
		if (year_i < 1900 || month_i < 1 || month_i > 12 || !day_i) return false;
		return day_i <= days(year_i, month_i);
	}


	NaiveDate::weekday_t NaiveDate::weekday() const
	{
		FCET(valid(), std::logic_error, "Internal date not valid");

		/// Broken Down Time
		std::tm bdt {};
		bdt.tm_year = year_ - 1900;
		bdt.tm_mon = month_ - 1;
		bdt.tm_mday = day_;
		bdt.tm_hour = 12;
		bdt.tm_min = 0;
		bdt.tm_sec = 0;
		bdt.tm_isdst = -1;

		std::tm * tmptr = nullptr;
		std::time_t epoch = std::mktime(&bdt);
		tmptr = std::gmtime(&epoch);
		return static_cast<NaiveDate::weekday_t>(tmptr->tm_wday);
	}

	uint16_t NaiveDate::year() const
	{
		FCET(year_, std::logic_error, "Year not initialized yet");
		return year_;
	}

	void NaiveDate::year(const uint16_t year_i)
	{
		FCET(year_i >= 1900, std::invalid_argument, "Year to set should be 1900 or later");
		year_ = year_i;
	}

} // Bren namespace
