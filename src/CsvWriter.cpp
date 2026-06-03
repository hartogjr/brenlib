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

#include <stdexcept>
#include <utf8.h>
#include <bren/CsvWriter.hpp>
#include <bren/commondefs.hpp>
#include <bren/Logger.hpp>

namespace Bren {

	CsvWriter::CsvWriter(const CsvWriter::csvSettings_t & csvSettings_i)
	: column_(0), columns_(0), lines_(0), out_(nullptr), settings_(csvSettings_i)
	{ }

	void CsvWriter::destination(std::ostream * destination_i)
	{
		FCET(destination_i != nullptr, std::invalid_argument, "Unable to write CSV to NULL pointer");
		FCET(out_ == nullptr || destination_i == out_, std::logic_error,
			"Won't overwrite existing destination pointer {:p} with different destination pointer {:p}",
			static_cast<void *>(out_), static_cast<void *>(destination_i));
		out_ = destination_i;
		out_->exceptions(std::ios_base::eofbit | std::ios_base::failbit | std::ios_base::badbit);
	}

	std::string CsvWriter::doubleQuote_(const std::string & str_i)
	{
		// All preconditions have already been checked by the methods that call this method
		size_t pos = 0;
		std::string rv = str_i;

		while ( (pos = rv.find('"', pos)) != std::string::npos) {
			pos++;
			rv.insert(pos++, 1, '"');
		}

		return rv;
	}

	size_t CsvWriter::endRow()
	{
		FCET(out_ != nullptr, std::logic_error, "Destination pointer for output not set yet, call destination() method first");
		if (columns_) {
			// Amount of fields is already known
			if (!column_) {
				// Already wrote end of row, return number of fields
				return columns_;
			}
			FCET(column_ == columns_, std::out_of_range,
				"Current amount of written columns ({}) is not equal to number of fields in CSV ({})",
				column_, columns_);
		} else {
			// Fields is not known yet, set it
			columns_ = column_;
		}

		if (settings_.carriageReturn) out_->write("\r\n", 2);
		else out_->put('\n');
		column_=0;
		lines_++;

		return columns_;
	}

	size_t CsvWriter::field(const std::string & field_i)
	{
		FCET(out_ != nullptr, std::logic_error, "Destination pointer for output not set yet, call destination() method first");
		FCET(!settings_.forceHeader || lines_, std::logic_error, "The output CSV should contain a header but it was not yet written");
		return quotedField_(field_i);
	}

	size_t CsvWriter::fields(const std::vector<std::string> & fields_i)
	{
		FCET(out_ != nullptr, std::logic_error, "Destination pointer for output not set yet, call destination() method first");
		FCET(!settings_.forceHeader || lines_, std::logic_error, "The output CSV should contain a header but it was not yet written");
		return writeFields_(fields_i);
	}

	size_t CsvWriter::headers(const std::vector<std::string> & names_i)
	{
		FCET(out_ != nullptr, std::logic_error, "Destination pointer for output not set yet, call destination() method first");
		FCET(lines_ == 0, std::logic_error, "Header has already been written");
		return writeFields_(names_i);
	}

	size_t CsvWriter::header(const std::string & name_i)
	{
		FCET(out_ != nullptr, std::logic_error, "Destination pointer for output not set yet, call destination() method first");
		FCET(lines_ == 0, std::logic_error, "Header has already been written");
		return quotedField_(name_i);
		return UINT16_MAX;
	}

	size_t CsvWriter::quotedField_(const std::string & field_i)
	{
		// All preconditions have already been checked by the methods that call this method, except UTF-8 ;-)
		std::string bad = "\r\n", toPrint = "";
		size_t pos = 0;
		bad += settings_.fieldSep;

		FCET(utf8::is_valid(field_i.begin(), field_i.end()), std::invalid_argument, "Given field >{}< is not a valid UTF-8 encoded string", field_i);

		switch (settings_.quoting) {
			case never:
				pos = field_i.find_first_of(bad);
				FCET(pos == std::string::npos, std::runtime_error,
					"Field >{}< contains character at zero-based position {} that needs mandatory quoting",
					field_i, pos);
				toPrint = field_i;
				break;

			case always:
				toPrint += doubleQuote_(field_i);
				toPrint.insert(0, 1, '"');
				toPrint.push_back('"');
				break;

			case needed:
				bad += '"';
				toPrint = doubleQuote_(field_i);
				if (toPrint.find_first_of(bad) != std::string::npos) {
					toPrint.insert(0, 1, '"');
					toPrint.push_back('"');
				}
				break;
		}

		if (column_) out_->put(settings_.fieldSep);
		out_->write(toPrint.data(), toPrint.size());
		column_++;
		if (columns_ && column_ == columns_) return endRow();
		else return column_;
	}

	size_t CsvWriter::writeFields_(const std::vector<std::string> & row_i)
	{
		if (columns_) {
			FCET(column_ + row_i.size() <= columns_, std::out_of_range,
			"Current column {} + fields in row ({}) would exceed column count {}",
			column_, row_i.size(), columns_);
		}
		for (const auto & fld : row_i) quotedField_(fld);
		return column_?column_:columns_;
	}

	const CsvWriter::csvSettings_t CsvWriter::PayPal = {
		',',    // fieldSep
		false,  // carriageReturn
		true,   // forceHeader
		always  // quoting
	};

	const CsvWriter::csvSettings_t CsvWriter::TicketMatic = {
		';',    // fieldSep
		false,  // carriageReturn
		true,   // forceHeader
		needed  // quoting
	};

} // Bren namespace
