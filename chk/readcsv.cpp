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

#include <sstream>
#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/readCSV.hpp>

#define CHECKNAME readcsvCheck

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

class CHECKNAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CHECKNAME);
	CPPUNIT_TEST(validcsv);
	CPPUNIT_TEST_SUITE_END();

	public:

	void validcsv() {
		using namespace std::string_literals;
		std::stringstream sss;

		sss << "First;Second;Third\nUnquoted;\"QUOTED\";\"ESC\"\"QUOT\"\n";
		Bren::Table<std::string> t;
		CPPUNIT_ASSERT_NO_THROW(t = Bren::readCSV(sss, ';'));
		CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(3), t.columns());
		CPPUNIT_ASSERT_EQUAL(2U, t.rows());
		CPPUNIT_ASSERT_EQUAL("First"s, t.cell(0, 0));
		CPPUNIT_ASSERT_EQUAL("Second"s, t.cell(1, 0));
		CPPUNIT_ASSERT_EQUAL("Third"s, t.cell(2, 0));
		CPPUNIT_ASSERT_EQUAL("Unquoted"s, t.cell(0, 1));
		CPPUNIT_ASSERT_EQUAL("QUOTED"s, t.cell(1, 1));
		CPPUNIT_ASSERT_EQUAL("ESC\"QUOT"s, t.cell(2, 1));
	}

};

#undef CHECKNAME
