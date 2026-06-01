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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/Table.hpp>

#define CHECKNAME tableCheck

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

class CHECKNAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CHECKNAME);
	CPPUNIT_TEST(construction);
	CPPUNIT_TEST(columns);
	CPPUNIT_TEST(rows);
	CPPUNIT_TEST(cells);
	CPPUNIT_TEST_SUITE_END();

	public:

	void construction() {
		Bren::Table<int> t;

		// Check that column count is zero
		CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), t.columns());

		// Check that row count is zero
		CPPUNIT_ASSERT_EQUAL(0U, t.rows());

		// Check that all methods throw exceptions when no column count has been set yet
		CPPUNIT_ASSERT_THROW(t.rows(0), std::logic_error);
		CPPUNIT_ASSERT_THROW(t.rows(3), std::logic_error);
		CPPUNIT_ASSERT_THROW(t.cell(0, 0), std::logic_error);
		CPPUNIT_ASSERT_THROW(t.cell(8, 0), std::logic_error);
		CPPUNIT_ASSERT_THROW(t.cell(0, 5), std::logic_error);
		CPPUNIT_ASSERT_THROW(t.cell(4, 7), std::logic_error);
	}

	void columns() {
		Bren::Table<int> t;

		// Zero columns not allowed
		CPPUNIT_ASSERT_THROW(t.columns(0), std::invalid_argument);

		// Column count remains zero after throw
		CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(0), t.columns());

		// Regular column set
		CPPUNIT_ASSERT_NO_THROW(t.columns(2));

		// Check that column count was set
		CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(2), t.columns());

		// Not allowed to change columns
		CPPUNIT_ASSERT_THROW(t.columns(3), std::logic_error);

		// Column count should not have changed
		CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(2), t.columns());

		// Can re-set to same number of columns
		CPPUNIT_ASSERT_NO_THROW(t.columns(2));

		// Column count should not have changed
		CPPUNIT_ASSERT_EQUAL(static_cast<uint16_t>(2), t.columns());
	}

	void rows() {
		Bren::Table<int> t;

		CPPUNIT_ASSERT_NO_THROW(t.columns(2));

		// Assure row count is still zero after column set
		CPPUNIT_ASSERT_EQUAL(0U, t.rows());

		// Clearing data throws nothing
		CPPUNIT_ASSERT_NO_THROW(t.rows(0));
		CPPUNIT_ASSERT_EQUAL(0U, t.rows());

		// Allocate a couple of rows
		CPPUNIT_ASSERT_NO_THROW(t.rows(3));
		CPPUNIT_ASSERT_EQUAL(3U, t.rows());

		// Clear allocated rows
		CPPUNIT_ASSERT_NO_THROW(t.rows(0));
		CPPUNIT_ASSERT_EQUAL(0U, t.rows());

		// Allocate different rows
		CPPUNIT_ASSERT_NO_THROW(t.rows(5));
		CPPUNIT_ASSERT_EQUAL(5U, t.rows());

		// Allocation of same rowcount succeeds
		CPPUNIT_ASSERT_NO_THROW(t.rows(5));
		CPPUNIT_ASSERT_EQUAL(5U, t.rows());

		// Allocation of higher rowcount succeeds
		CPPUNIT_ASSERT_NO_THROW(t.rows(8));
		CPPUNIT_ASSERT_EQUAL(8U, t.rows());

		// Allocation of lower rowcount succeeds
		CPPUNIT_ASSERT_NO_THROW(t.rows(4));
		CPPUNIT_ASSERT_EQUAL(4U, t.rows());
	}

	void cells()
	{
		Bren::Table<int> t;

		CPPUNIT_ASSERT_NO_THROW(t.columns(3));

		// Check bounds
		CPPUNIT_ASSERT_THROW(t.cell(5, 0), std::out_of_range);
		CPPUNIT_ASSERT_EQUAL(0U, t.rows());
		CPPUNIT_ASSERT_THROW(t.cell(3, 2), std::out_of_range);
		CPPUNIT_ASSERT_EQUAL(0U, t.rows());

		// Work on first cell
		t.cell(0, 0);
		CPPUNIT_ASSERT_EQUAL(1U, t.rows());
		t.cell(0, 0) = 42;
		CPPUNIT_ASSERT_EQUAL(1U, t.rows());
		int r = t.cell(0, 0);
		CPPUNIT_ASSERT_EQUAL(42, r);

		// Create some rows through cell referencing
		t.cell(2, 6);
		CPPUNIT_ASSERT_EQUAL(7U, t.rows());
	}

};

#undef CHECKNAME
