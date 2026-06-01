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

#include <iostream>
#include <sstream>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/Observer.hpp>
#include <bren/Observable.hpp>

#define CHECKNAME ObservingCheck

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

namespace Bren
{
	bool operator==(const std::weak_ptr<Bren::Observer<bool>> lhs, const std::weak_ptr<Bren::Observer<bool>> rhs)
	{
		// Left and right shared pointer
		auto lsp = lhs.lock();
		auto rsp = rhs.lock();

		if (!lsp && !rsp) return true;
		if (lsp && rsp) return lsp.get() == rsp.get();
		return false;
	}

	std::ostringstream & operator<<(std::ostringstream & oss_i, const std::weak_ptr<Bren::Observer<bool>> & wp_i)
	{
		auto sp = wp_i.lock();
		if (!sp) oss_i << "(expired weak ptr)";
		else oss_i << static_cast<void *>(sp.get());
		return oss_i;
	}

} // Bren namespace

class CHECKNAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CHECKNAME);
	CPPUNIT_TEST(all);
	CPPUNIT_TEST_SUITE_END();

	protected:
		class Watcher : public Bren::Observer<bool> {
			public:
				// Keep track whether we received an update
				bool updated;

				// Received value on update
				bool value;

				Watcher() : updated(false), value(true) { }

				// needed because of virtual class
				virtual ~Watcher() = default;

				virtual void observableChanged(const bool & value_i) override {
					updated = true;
					value = value_i;
				}
		};

		class Topic : public Bren::Observable<bool> {
			public:
				Topic(bool init_i)
					: Bren::Observable<bool>::Observable(init_i)
				{ }
		};

	public:
		void setUp() {
		}

		void tearDown() {
		}

		void all() {
			Topic t(false);

			// Check that the initialization value gets through. We do a
			// double test to assure that the compiler default value for
			// booleans does not accidentally make the test succeed.
			CPPUNIT_ASSERT_EQUAL(false, t.get());
			{
				Topic t2(true);
				CPPUNIT_ASSERT_EQUAL(true,  t2.get());
			}

			// Check that nothing is being observed by default
			CPPUNIT_ASSERT_EQUAL(0UL, t.observers().size());

			// Check that setting same and different value without observer works
			CPPUNIT_ASSERT_EQUAL(false, t.set(false));
			CPPUNIT_ASSERT_EQUAL(true, t.set(true));
			CPPUNIT_ASSERT_EQUAL(false, t.set(true));

			// Separate scope for testing observer destructor
			{
				auto w = std::make_shared<Watcher>();

				// Start observing t
				CPPUNIT_ASSERT(t.addObserver(w->weak_from_this()));

				// Check that t is being watched by w
				CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size());
				CPPUNIT_ASSERT_EQUAL(w->weak_from_this(), *(t.observers().begin()));

				// Can't add empty observers
				CPPUNIT_ASSERT_EQUAL(false, t.addObserver(nullptr));
				CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size());
				// (Empty Weak Pointer)
				std::weak_ptr<Bren::Observer<bool>> ewp;
				CPPUNIT_ASSERT_EQUAL(false, t.addObserver(ewp));
				CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size());

				// Verify that observer won't be added twice
				CPPUNIT_ASSERT_EQUAL(false, t.addObserver(w->weak_from_this()));
				CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size());
				CPPUNIT_ASSERT_EQUAL(false, t.addObserver(w->weak_from_this()));
				CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size());

				// Check that watcher has not been updated yet
				CPPUNIT_ASSERT_EQUAL(false, w->updated);

				// Check that setting same value does not trigger update
				CPPUNIT_ASSERT_EQUAL(false, t.set(true));
				CPPUNIT_ASSERT_EQUAL(false, w->updated);

				// Check that setting different value does trigger update
				CPPUNIT_ASSERT_EQUAL(true, t.set(false));
				CPPUNIT_ASSERT_EQUAL(true, w->updated);
				CPPUNIT_ASSERT_EQUAL(false, w->value);
				w->updated = false;

				// Can't remove empty observers
				CPPUNIT_ASSERT_EQUAL(false, t.removeObserver(nullptr));
				CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size());
				CPPUNIT_ASSERT_EQUAL(false, t.removeObserver(ewp));
				CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size());

				// Check that removing an observer works
				CPPUNIT_ASSERT_EQUAL(true, t.removeObserver(w->weak_from_this()));
				CPPUNIT_ASSERT_EQUAL(0UL, t.observers().size());
				CPPUNIT_ASSERT_EQUAL(true, t.set(true));
				CPPUNIT_ASSERT_EQUAL(false, w->updated);

				// Observe again for destructor
				CPPUNIT_ASSERT_EQUAL(true, t.addObserver(w->weak_from_this()));
			}

			// Watcher destructed, check that t still works
			CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size()); // Empty Observer
			CPPUNIT_ASSERT_EQUAL(false, t.set(true));
			CPPUNIT_ASSERT_EQUAL(1UL, t.observers().size()); // Empty Observer
			CPPUNIT_ASSERT_EQUAL(true, t.set(false));
			CPPUNIT_ASSERT_EQUAL(0UL, t.observers().size()); // Empty Observer cleaned up
		}

};
