/** @author   Bren de Hartog <bren@dehartog.name>
 * @copyright Copyright (c) 2026, Bren de Hartog. All rights reserved.
 * @license   This project is licensed under the 3-clause BSD license:
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
#include <string>

#ifdef COOLENUM
#undef COOLENUM_BEGIN
#undef COOLENUM
#undef COOLENUM_END
#endif

#define COOLENUM_BEGIN(NAME, KEYTYPE) \
	const std::string & NAME ## _ (const KEYTYPE p_key) { \
		NAME ## _fillenummaps(); \
		auto i = NAME ## _k2s.find(p_key); \
		if (i == NAME ## _k2s.end()) { \
			throw std::out_of_range("Requested enum key not found in " #NAME); \
		} \
		return i->second; \
	} \
	NAME NAME ## _ (const std::string & p_str) { \
		NAME ## _fillenummaps(); \
		auto i = NAME ## _s2k.find(p_str); \
		if (i == NAME ## _s2k.end()) { \
			throw std::out_of_range("Requested enum value not found in " #NAME); \
		} \
		return (NAME) i->second; \
	} \
	std::unordered_map<KEYTYPE, std::string> NAME ## _k2s; \
	std::unordered_map<std::string, KEYTYPE> NAME ## _s2k; \
	void NAME ## _fillenummaps() { \
		std::unordered_map<KEYTYPE, std::string> * k2s = & NAME ## _k2s; \
		std::unordered_map<std::string, KEYTYPE> * s2k = & NAME ## _s2k; \
		if (k2s->empty()) {

#define COOLENUM(VALUE, INDEX) \
	(*k2s)[INDEX] = #VALUE; \
	(*s2k)[#VALUE] = INDEX;

#define COOLENUM_END() \
	} \
	}
