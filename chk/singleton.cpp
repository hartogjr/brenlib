/* Copyright (c) 2025 Bren de Hartog <bren@dehartog.name>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE. */

#include <memory>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/Singleton.hpp>

#define CHECKNAME SingletonCheck

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

class CHECKNAME : public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(CHECKNAME);
		CPPUNIT_TEST(instance);
		CPPUNIT_TEST_SUITE_END();

		class DummySingleton : public Bren::Singleton<DummySingleton> {
				friend class Bren::Singleton<DummySingleton>;

				DummySingleton()
					: dummy(true) {}

			protected:
				bool dummy;

			public:
				bool check()
				{
					return dummy;
				}
		};

	public:
		void instance()
		{
			DummySingleton *ds = nullptr;

			CPPUNIT_ASSERT_EQUAL(false, DummySingleton::is_constructed());
			CPPUNIT_ASSERT_NO_THROW(ds = DummySingleton::instance());
			CPPUNIT_ASSERT_EQUAL(true, DummySingleton::is_constructed());
			CPPUNIT_ASSERT(ds->check());
			CPPUNIT_ASSERT_NO_THROW(DummySingleton::close());
			CPPUNIT_ASSERT_EQUAL(false, DummySingleton::is_constructed());
		}

};
