/** @author   Bren de Hartog <bren@dehartog.name>
 * @copyright Copyright (c) 2026, Bren de Hartog. All rights reserved.
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

#include <filesystem>
#include <iterator>
#include <stdexcept>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/Child.hpp>

#define CHECKNAME ChildCheck

#define FDCOUNT std::distance(std::filesystem::directory_iterator("/proc/self/fd"), \
		std::filesystem::directory_iterator())

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

class CHECKNAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CHECKNAME);
	CPPUNIT_TEST(exits);
	CPPUNIT_TEST(fdleaks);
	CPPUNIT_TEST_SUITE_END();

	public:
	void exits()
	{
		{
			Bren::Child rettrue;
			rettrue.command("/usr/bin/true");
			CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0), rettrue.run());
		}
		{
			Bren::Child retfalse;
			retfalse.command("/usr/bin/false");
			CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(1), retfalse.run());
		}
	}

	void fdleaks()
	{
		{
			long fds_pre = FDCOUNT;
			Bren::Child nocaps;
			nocaps.command("/usr/bin/true");
			CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0), nocaps.run());
			CPPUNIT_ASSERT_EQUAL(fds_pre, FDCOUNT);
		}
		{
			long fds_pre = FDCOUNT;
			Bren::Child capout;
			capout.command("/usr/bin/true");
			capout.capture(true, false);
			CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0), capout.run());
			CPPUNIT_ASSERT_EQUAL(fds_pre, FDCOUNT);
		}
		{
			long fds_pre = FDCOUNT;
			Bren::Child capboth;
			capboth.command("/usr/bin/true");
			capboth.capture(true, true);
			CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0), capboth.run());
			CPPUNIT_ASSERT_EQUAL(fds_pre, FDCOUNT);
		}
	}

};

#undef CHECKNAME
