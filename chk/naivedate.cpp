/** @author   Bren de Hartog <bren@dehartog.name>
 * @copyright Copyright (c) 2025, Bren de Hartog. All rights reserved.
 * @license   This project is licensed under 3-clause BSD license:
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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/NaiveDate.hpp>

#define CHECKNAME NaiveDateCheck

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

class CHECKNAME : public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(CHECKNAME);
		CPPUNIT_TEST(weekday);
		CPPUNIT_TEST_SUITE_END();

	public:
		void weekday()
		{
			Bren::NaiveDate nd;

			nd.year(2025);
			nd.month(11);
			nd.day(1);

			CPPUNIT_ASSERT_EQUAL(Bren::NaiveDate::Sat, nd.weekday());

			nd.day(2);
			CPPUNIT_ASSERT_EQUAL(Bren::NaiveDate::Sun, nd.weekday());
		}

};
