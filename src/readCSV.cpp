/* BSD 3-Clause License
 *
 * Copyright (c) 2025, Bren de Hartog <bren@dehartog.name>
 * All rights reserved.
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
#include <iostream>
#include <string>
#include <bren/readCSV.hpp>

#define ERR "Error at line "s + std::to_string(line) + ", character "s + std::to_string(pos) + ": "s

namespace Bren {

	Table<std::string> readCSV(std::istream & stream_i, const char separator_i)
	{
		using namespace std::string_literals;

		Table<std::string> t;
		std::vector<std::string> h; // Header
		bool readingHeader = true;
		bool quoted = false;
		char c;
		std::string f; // Field with data
		uint16_t col = 0;
		uint32_t row = 0;
		size_t line = 1;
		size_t pos = 1;

		enum last_e : uint8_t {
			beginQuote,
			endQuote,
			fieldData,
			separator,
			startOfRecord
		};

		last_e last = startOfRecord;

		auto fieldReady = [&]() {
			if (readingHeader) {
				h.push_back(f);
			}
			else {
				t.cell(col, row) = f;
			}
			f.clear();
		};

		// Disable exceptions for EOF on the stream, because we need to be able to handle EOF properly.
		// If something bad happens other than EOF, throw exception.
		stream_i.exceptions(std::istream::badbit);

		while (stream_i.good()) {
			stream_i.read(&c, 1);
			if (stream_i.eof()) c = EOF;
			else pos++;

			// Skip valid but unused 8-bit characters at start of CSV
			if (readingHeader && last == startOfRecord && col == 0 && row == 0 && c != EOF && c & 0x80) continue;

			// If a field is quoted, only check for EOF, escaped quotes or end quote.
			// Store everything else (including separators and newlines) in the field data.
			// Do check newlines to keep track of character position.
			if (quoted) {
				switch (c) {
					case EOF:
						throw std::runtime_error(ERR + "Encountered EOF while reading quoted field");

					case '\r':
						if (stream_i.peek() == '\n') {
							f.push_back(c);
							// Swallow \r\n
							stream_i.read(&c, 1);
						}
						// Fallthrough
					case '\n':
						f.push_back(c);
						line++; pos = 0;
						last = fieldData;
						continue;

					case '"':
						if (stream_i.peek() == '"') {
							// Escaped quote
							stream_i.read(&c, 1);
							pos++;
							f.push_back(c);
							last = fieldData;
						} else {
							// End quote. The field is stored on the next separator or end of record
							quoted = false;
							last = endQuote;
						}
						continue;

					default:
						f.push_back(c);
						last = fieldData;
						continue;
				}
			}

			// From here on, we are not reading a quoted field, so parse normally
			if (c == '\r' || c == '\n' || c == EOF) {
				// End of record
				if (c == '\r' && stream_i.peek() == '\n') {
					// Swallow \r\n
					stream_i.read(&c, 1);
					pos++;
				}
				switch (last) {
					case beginQuote:
						throw std::runtime_error(ERR + "Found End Of Record just after begin quote");

					case endQuote:
					case fieldData:
					case separator:
						fieldReady();
						break;

					case startOfRecord:
						if (stream_i.peek() != EOF) throw std::runtime_error(ERR + "Found empty line");
						else return t;
				}

				if (readingHeader) {
					if (h.empty()) {
						throw std::runtime_error(ERR + "Empty line at beginning, unable to determine number of columns"s);
					}
					// Both column adressing and count are
					// uint16_t, so max is UINT16_MAX - 1
					if (h.size() >= UINT16_MAX) {
						throw std::out_of_range(ERR + "Encountered too many columns "s + std::to_string(h.size()) +
							", maximum is " + std::to_string(UINT16_MAX-1));
					}
					t.columns(static_cast<uint16_t>(h.size()));
					for (col = 0; col < h.size(); col++) {
						t.cell(col, 0) = h.at(col);
					}
					h.clear();
					row = 1;
					col = 0;
					pos = 0;
					readingHeader = false;
				} else {
					if (col > 0 && !f.empty() && col != t.columns() - 1) {
						throw std::runtime_error(ERR + "Found EndOfRecord after "s + std::to_string(col+1) +
							" column(s), but column count should be "s + std::to_string(t.columns()));
					}
					col = 0;
					if (row == UINT32_MAX) {
						throw std::runtime_error(ERR + "Input data has more than "s + std::to_string(UINT32_MAX) + " rows");
					}
					row++;
				}
				line++;
				pos = 0;
				last = startOfRecord;
				continue;
			}

			// Remember, quoted = false here
			if (c == '"') {
				switch (last) {
					case beginQuote:
					case endQuote:
						throw std::runtime_error(ERR + "Found quote character after quote, but not quoted field?");

					case fieldData:
						// Include sole " in unquoted field
						f.push_back(c);
						continue;

					case separator:
					case startOfRecord:
						quoted = true;
						f.clear();
						continue;
				}
				continue;
			}

			// Remember, quoted = false here
			if (c == separator_i) {
				fieldReady();
				last = separator;
				if (!readingHeader) {
					if (col >= t.columns() - 1) {
						throw std::runtime_error(ERR + "Field count "s + std::to_string(col+1) +
							" would exceed column count "s + std::to_string(t.columns()));
					}
					col++;
				}
				continue;
			}

			// The default case for c:
			f.push_back(c);
			last = fieldData;
		}

		return t;
	}

} // Bren namespace
