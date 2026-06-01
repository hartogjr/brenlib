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

#include <bren/HeaderedTable.hpp>

namespace Bren {

	using namespace std::string_literals;
	using std::string;

	string & HeaderedTable::cell(const string & colname_i, const uint32_t row_i)
	{
		if (cols_ == 0) throw std::logic_error("Please set the number of columns first");
		if (rows() < 1) throw std::logic_error("No first row to read as header");
		if (colname_i.empty()) throw std::invalid_argument("Given column name is empty");
		if (hdrs_.empty()) readHeaders_();

		// Headers Iterator
		auto hi = hdrs_.find(colname_i);
		if (hi == hdrs_.end()) throw std::invalid_argument("Unknown column header \""s + colname_i + "\"");
		return Table<string>::cell(hi->second, row_i);
	}

	void HeaderedTable::makeHeadersUnique_()
	{
		for(uint16_t c = 0; c < cols_; c++) { // Column index
			for (uint16_t d = c+1; d < cols_; d++) { // Duplicate index
				if (Table<string>::cell(c, 0) == Table<string>::cell(d, 0)) {
					// Duplicate header found
					string uniqName = "";
					uint16_t a = 0; // header Addition to make it unique
					// Tempted to let "a" go to UINT16_MAX inclusive, but
					// then the loop would never end due to wrapping.
					while (a < UINT16_MAX) {
						uniqName = Table<string>::cell(d, 0) + "_" + std::to_string(a);
						for (uint16_t p = 0; p < cols_; p++) { // Potential new duplicate
							if (Table<string>::cell(p, 0) == uniqName) {
								// Altered column name is also a duplicate,
								// continue searching
								uniqName = "";
								break;
							}
						}
						// Unique new column name found
						if (uniqName.size()) break;
						a++;
					}

					if (uniqName.empty()) throw std::runtime_error("Column name "s + Table<string>::cell(c, 0) + " has duplicates, but unable to generate new unique name");
					Table<string>::cell(d, 0) = uniqName;
				}
			}
		}
	}

	void HeaderedTable::readHeaders_()
	{
		makeHeadersUnique_();
		hdrs_.clear();

		for (uint16_t c = 0; c < cols_; c++) {
			string hdrname = Table<string>::cell(c, 0);
			if (hdrname.empty()) {
				hdrs_.clear();
				throw std::runtime_error("Header at row 1, column "s + std::to_string(c+1) + " is empty");
			}
			hdrs_.insert(std::make_pair(hdrname, c));
		}
	}

} // Bren namespace
