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
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * vim:set ts=4 sw=4 noet tw=120: */

#pragma once

#include <cstdint>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

namespace Bren {

	/** Class that models a two-dimensional array, similar to a database
	 * table.
	 * The number of columns can't be altered when set, but the number of rows can change throughout the lifetime of the object.
	 * This class uses std::vector for storage, it only does some facade calculations to mimic a two-dimensional table.
	 */
	template <typename T>
	class Table
	{
		protected:
		/** Number of columns in this table. 0 if not yet specified. */
		uint16_t cols_;

		/** Data vector containing all cells, grouped by row.
		 * This means that data_[0] = the cell in row 0, column 0,
		 * data_[1] = row 0, column 1, data_[2] = row 0, column 2, etc. */
		std::vector<T> data_;

		public:
		/** Table constructor */
		Table() : cols_(0)
		{}

		/** Table destructor */
		virtual ~Table()
		{
			data_.clear();
		}

		/** Get the number of columns in this table.
		 * @returns number of columns, 0 if not yet specified. */
		virtual inline uint16_t columns() const { return cols_; }

		/** Set the number of columns in this table.
		 * Can only be specified once and should be greater than zero.
		 * @param columns_i Number of columns to allocate (>0)
		 * @throws std::invalid_argument when columns_i == 0
		 * @throws std::logic_error when number of columns was already set. */
		virtual void columns(const uint16_t columns_i)
		{
			using namespace std::string_literals;

			if (columns_i == 0) {
				throw std::invalid_argument("Number of columns not allowed to be 0");
			}
			if (cols_ != 0 && columns_i != cols_) {
				throw std::logic_error("Number of columns is already set to "s + std::to_string(cols_));
			}

			cols_ = columns_i;
		}

		/** Fetch a reference to a specific field, increasing the number of
		 * rows if necessary.
		 * @param column_i Column of field to fetch, zero based
		 * @param row_i Row of field to fetch, zero based
		 * @returns Reference to field
		 * @throws std::out_of_range if requested column is outside of
		 * column range. */
		virtual T & cell(const uint16_t column_i, const uint32_t row_i)
		{
			using namespace std::string_literals;

			if (cols_ == 0) {
				throw std::logic_error("Please set the number of columns first");
			}
			if (column_i >= cols_) {
				throw std::out_of_range("Requested column "s + std::to_string(column_i) +
					" should be less than the number of columns, which is "s + std::to_string(cols_));
			}
			if (row_i >= rows()) rows(row_i+1);

			return data_.at(row_i * cols_ + column_i);
		}

		/** Get the number of rows in this table.
		 * @returns Number of allocated rows */
		virtual inline uint32_t rows() const {
			if (cols_ == 0) return 0;
			else return static_cast<uint32_t>(data_.size() / cols_);
		}

		/** Set the number of rows, thereby either removing data in rows
		 * beyond the new rowcount or adding empty rows.
		 * @param rows_i New rowcount
		 * @throws std::logic_error when number of columns hasn't been set
		 * yet. */
		virtual void rows(const uint32_t rows_i)
		{
			if (cols_ == 0) {
				throw std::logic_error("Please set the number of columns first");
			}
			data_.resize(cols_ * rows_i);
		}
	};

} // Bren namespace
