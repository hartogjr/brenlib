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

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/functions.hpp>

#define CHECKNAME functionsCheck

using namespace std::string_literals;

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

class CHECKNAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CHECKNAME);
	CPPUNIT_TEST(hcerc2ascii);
	CPPUNIT_TEST(join);
	CPPUNIT_TEST_SUITE_END();

	public:

	void hcerc2ascii() {

		std::string s = "Plain old string without HTML Character Entity Reference Codes";

		CPPUNIT_ASSERT_EQUAL(s, Bren::htmlCharEntRefCodes2ascii(s));
		CPPUNIT_ASSERT_EQUAL("&amp stuff"s, Bren::htmlCharEntRefCodes2ascii("&amp stuff"));
		CPPUNIT_ASSERT_EQUAL("justsc;"s, Bren::htmlCharEntRefCodes2ascii("justsc;"));
		CPPUNIT_ASSERT_EQUAL("&stretchedlong;"s, Bren::htmlCharEntRefCodes2ascii("&stretchedlong;"));
		CPPUNIT_ASSERT_EQUAL("&amp;"s, Bren::htmlCharEntRefCodes2ascii("&amp;amp;"));
		CPPUNIT_ASSERT_EQUAL("&&&"s, Bren::htmlCharEntRefCodes2ascii("&amp;&amp;&"));
		CPPUNIT_ASSERT_EQUAL("&;"s, Bren::htmlCharEntRefCodes2ascii("&;"));
		CPPUNIT_ASSERT_EQUAL("&what;"s, Bren::htmlCharEntRefCodes2ascii("&what;"));
		CPPUNIT_ASSERT_EQUAL(" "s, Bren::htmlCharEntRefCodes2ascii("&nbsp;"));
		CPPUNIT_ASSERT_EQUAL("?"s, Bren::htmlCharEntRefCodes2ascii("&quest;"));
		CPPUNIT_ASSERT_EQUAL("<"s, Bren::htmlCharEntRefCodes2ascii("&lt;"));
	}

	void join() {
		std::vector<int> v;

		CPPUNIT_ASSERT_EQUAL(""s, Bren::join(v));
		CPPUNIT_ASSERT_EQUAL(""s, Bren::join(v, "sepstr"));
		CPPUNIT_ASSERT_EQUAL(""s, Bren::join(v, "><", [](auto d) { return std::to_string(d); }));

		v.push_back(42); v.push_back(23); v.push_back(119);

		CPPUNIT_ASSERT_EQUAL("42, 23, 119"s, Bren::join(v));
		CPPUNIT_ASSERT_EQUAL("4223119"s, Bren::join(v, ""s));
		CPPUNIT_ASSERT_EQUAL("42}{23}{119"s, Bren::join(v, "}{"s));
		CPPUNIT_ASSERT_EQUAL("420 230 1190"s, Bren::join(v, " ", [](auto d) { return std::to_string(d * 10); }));
		CPPUNIT_ASSERT_EQUAL("43, 24 or 120"s, Bren::join(v, ", ", [](auto d) { return std::to_string(d + 1); }, " or "));
	}

};

#undef CHECKNAME
