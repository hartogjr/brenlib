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

/** @file Miscellaneous convenience functions */

#pragma once

#include <string>
#include <unordered_map>
#include <time.h>

namespace Bren {

	/** Convert an epoch timestamp (number of seconds since January 1st, 1970)
	 * to an ISO8601-formatted date time string in the current timezone.
	 * The format is not completely standard, a space is used as a separator
	 * and no UTC offset is appended. The format is: YYYY-MM-DD hh:mm:ss
	 * @param timestamp_i Epoch timestamp, default is 0, meaning current time.
	 * @returns ISO8601-formatted time string. */
	std::string epoch2iso8601(const time_t timestamp_i = 0);

	/** Unordered map with HTML Character Entity Reference Codes to ASCII characters. */
	extern const std::unordered_map<std::string, const char> hcerc2asc;

	/** Convert HTML character entity reference codes to plain ASCII.
	 * @param str_i Input string with HTML character entity reference codes
	 * @returns String with HTML character entity reference codes replaced by their ascii counterparts. */
	std::string htmlCharEntRefCodes2ascii(const std::string & str_i);

	/** Join all items from something iterable into a string, separated by a
	 * given separator string.
	 * @param iterable_i Object to iterate over
	 * @param separator_i Separation string to use, default is ", "
	 * @param extrfunc_i Extraction function to convert item to string,
	 * default is &std::to_string
	 * @param lastsep_i Last separator to use. Facilitates "X", "Y" or "Z".
	 * Default is "", meaning @p separator_i (so last separator can't
	 * actually be the empty string)
	 * @returns All items from @p iterable_i concatenated with @p
	 * separator_i as separator string. */
	template <typename IterableObjType>
	std::string join(
		const IterableObjType & iterable_i,
		const std::string & separator_i = ", ",
		std::string (*extrfunc_i)(const typename IterableObjType::value_type) = &std::to_string,
		const std::string & lastsep_i = ""
	) {
		std::string rv = ""; // Return Value
		if (iterable_i.empty()) return rv;
		auto i = iterable_i.begin();
		rv = extrfunc_i(*i);
		for (i++; i != iterable_i.end(); i++) {
			auto j = i;
			j++;
			if (j == iterable_i.end() && lastsep_i != "") rv += lastsep_i;
			else rv += separator_i;
			rv += extrfunc_i(*i);
		}
		return rv;
	}

} // Bren namespace
