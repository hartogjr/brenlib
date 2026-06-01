#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <bren/Logger.hpp>

namespace Bren
{
	/** CSV writer that conforms to the TicketMatic export specification by default.
	 * This CSV writer is not thread-safe, since it would be useless to let
	 * two or more different threads write to the same CSV output stream simultaneously. */
	class CsvWriter
	{
		public:
		enum csvQuoting_e : uint8_t {
			never,  //< Never quote fields
			always, //< Always quote fields
			needed  //< Only quote fields when they contain CSV-interpreted characters
		};

		typedef struct csvSettings_s {
			char fieldSep;        /// The CSV field separator
			bool carriageReturn;  /// Wether to use carriage returns in line endings
			bool forceHeader;     /// Whether to force outputting an initial header line
			csvQuoting_e quoting; /// When to quote fields
		} csvSettings_t;

		static const csvSettings_t TicketMatic;
		static const csvSettings_t PayPal;

		private:
		/// Assignment construction not allowed
		CsvWriter & operator=(const CsvWriter & obj_i) = delete;

		/// Copy construction not allowed
		CsvWriter(const CsvWriter & obj_i) = delete;

		protected:
		/// Columns written for current row
		size_t column_;

		/// Number of columns in output CSV, 0 if not yet known
		size_t columns_;

		/// Lines written to output stream
		size_t lines_;

		/// Output stream to write to
		std::ostream * out_;

		/// Settings to use for CSV output
		csvSettings_t settings_;

		/** Double quote the given string where necessary.
		 * @param str_i String to double quote
		 * @returns Double quoted string */
		std::string doubleQuote_(const std::string & str_i);

		/** Write an optionally quoted field with separators and update internal counters
		 * @param field_i String to write
		 * @returns Column number the field was written as (1-based)
		 * @throws std::invalid_argument when field is not a valid UTF-8 string
		 * @throws std::runtime_error when quoting was disabled but necessary */
		size_t quotedField_(const std::string & field_i);

		/** Write multiple fields, uptil the maximum number of columns, if set.
		 * @param fields_i fields to write
		 * @returns Last column number that was written (1-based)
		 * @throws std::invalid_argument when a field is not a valid UTF-8 string
		 * @throws std::out_of_range when the fields would cross a row boundary */
		size_t writeFields_(const std::vector<std::string> & fields_i);

		public:
		/** CsvWriter Constructor with settings.
		 * @param csvSettings Settings to use for CSV output */
		CsvWriter(const csvSettings_t & csvSettings_i = TicketMatic);

		/// Destructor
		~CsvWriter() = default;

		/** Get last written column.
		 * @returns The last written column, 1-based. 0 means we are at the beginning of a new row. */
		inline size_t column() const { return column_; }

		/** Get number of columns.
		 * @returns Total number of columns in output, 0 if not yet known. */
		inline size_t columns() const { return columns_; }

		/** Set the destination of the CSV output.
		 * @param destination_i Pointer to an output stream
		 * @throws std::invalid_argument when destination_i is NULL
		 * @throws std::logic_error When destination was already set. */
		void destination(std::ostream * destination_i = &std::cout);

		/** Terminate current row.
		 * This method only needs to be called after the first row to
		 * determine the number of columns in the output CSV. After the first
		 * row, this class will end rows automatically.
		 * @returns Determined field count
		 * @throws std::out_of_range when the column count doesn't match
		 * with a previous setting. */
		size_t endRow();

		/** Write a single field.
		 * @param field_i Field to write
		 * @returns Column number the field was written as (1-based)
		 * @throws std::invalid_argument when field is not a valid UTF-8 string
		 * @throws std::logic_error when headers were forced and not written
		 * @throws std::runtime_error when quoting was disabled but necessary */
		size_t field(const std::string & field_i);

		/** Write multiple fields.
		 * @param fields_i fields to write
		 * @returns Last column number that was written (1-based)
		 * @throws std::invalid_argument when a field is not a valid UTF-8 string
		 * @throws std::logic_error when headers were forced and not yet written
		 * @throws std::out_of_range when the fields would cross a row boundary */
		size_t fields(const std::vector<std::string> & fields_i);

		/** Write a single header field.
		 * @param name_i Header field as column name to write
		 * @returns Column number the header field was written as (1-based)
		 * @throws std::logic_error when header has already been written */
		size_t header(const std::string & name_i);

		/** Write multiple header fields
		 * @param names_i Field names
		 * @returns Last column number that was written (1-based)
		 * @throws std::logic_error when header is already written */
		size_t headers(const std::vector<std::string> & names_i);

		/** Get the number of complete lines written to CSV output.
		 * This is equal to the number of newlines written.
		 * @returns Number of lines written. */
		inline size_t lines() const { return lines_; }
	}; // CsvWriter class

} // Bren namespace
