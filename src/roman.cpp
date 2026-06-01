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
#include <map>
#include <string>
#include <bren/roman.hpp>

namespace Bren {

	std::string toRoman(const uint16_t num)
	{
		uint16_t rem = num; // Remainder of num to convert
		std::string out;  // Output variable
		std::map<uint16_t, char> rn; // Roman numerals


		rn[1000] = 'M';
		rn[500] = 'D';
		rn[100] = 'C';
		rn[50] = 'L';
		rn[10] = 'X';
		rn[5] = 'V';
		rn[1] = 'I';

		/* An important discovery I made:
		 * The 5* numbers (V, L and D) are never used as a prefix to substract
		 * it from the following character. So only the 1* numbers (I, X and
		 * C) are substracted from the following character. Also, at most
		 * one substraction is done. */

		// Walk the map in reverse order
		for (auto i = rn.rbegin(); i != rn.rend(); i++) {
//			std::cout << "Checking roman numerals map at " << i->first << " = " << i->second << std::endl;

			while (rem >= i->first) {
//				std::cout << "Filling up with " << i->first << " = " << i->second << std::endl;
				out += i->second; rem -= i->first;
			}
			for (auto j = rn.begin(); j->first != i->first; j++) {
//				std::cout << "Checking substractions with " << j->first << " = " << j->second << std::endl;
				if (j->first != 100 && j->first != 10 && j->first != 1) continue;
				if (rem >= i->first - j->first) {
//					std::cout << "Found substraction for remaining " << rem << " with " << i->first << " - " << j->first << std::endl;
					out += j->second;
					out += i->second;
					rem -= (i->first - j->first);
					break;
				}
			}
		}

		return out;
	}

} // Bren namespace
