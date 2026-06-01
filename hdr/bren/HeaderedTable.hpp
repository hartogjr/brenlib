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

#pragma once

#include <string>
#include <unordered_map>
#include <bren/Table.hpp>

namespace Bren {

	class HeaderedTable : public Table<std::string>
	{
		protected:
		/// Map of strings to column indices to mimic associative columns.
		std::unordered_map<std::string, uint16_t> hdrs_;

		/** Make header names unique by adding digits when duplicates are
		 * found. */
		void makeHeadersUnique_();

		/** Read the column headers for associative column addressing. */
		void readHeaders_();

		public:
		/// Constructor
		HeaderedTable() = default;

		/// Copy constructor from superclass
		HeaderedTable(const Table<std::string> & obj_i)
		: Table(obj_i)
		{}

		/// Destructor
		virtual ~HeaderedTable() = default;

		/// Explicitly inherit the Table::cell() method, otherwise it is hidden.
		using Table<std::string>::cell;

		/** Reference a cell by row number and column name.
		 * @param colname_i Column name to reference.
		 * @param row_i Row number to reference, header is row 0. */
		virtual std::string & cell(const std::string & colname_i, const uint32_t row_i);

	};

} // Bren namespace
