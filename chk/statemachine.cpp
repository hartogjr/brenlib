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

#include <cstdint>
#include <memory>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/StateMachine.hpp>

#define CHECKNAME StateMachineCheck

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

class CHECKNAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CHECKNAME);
	CPPUNIT_TEST(basics);
	CPPUNIT_TEST(funccall);
	CPPUNIT_TEST_SUITE_END();

	protected:
	/// State machine instance for easy setup and teardown
	std::unique_ptr<Bren::StateMachine<checkState, checkEvent, CHECKNAME>> smup;

	public:
	enum class checkState : uint8_t {
		Start,
		Inter,
		End
	};

	enum class checkEvent : uint8_t {
		Event,
		Quit
	};

	friend class Bren::StateMachine<checkState, checkEvent, CHECKNAME>;

	void setUp()
	{
		smup.reset(
			new Bren::StateMachine<checkState, checkEvent, CHECKNAME>(
				checkState::Start, checkState::End
			)
		);
		smup->transition(checkState::Start, checkEvent::Event, checkState:Inter, nullptr, nullptr);
		smup->transition(checkState::Start, checkEvent::Quit, checkState:End, nullptr, nullptr);
		smup->transition(checkState::Inter, checkEvent::Quit, checkState:End, nullptr, nullptr);
	}

	void tearDown()
	{
		smup.reset();
	}

	void basics()
	{
		// Check start state
		CPPUNIT_ASSERT_EQUAL(checkState::Start, smup->state());

		// Don't allow transitions from end state
		CPPUNIT_ASSERT_THROW(
			smup->transition(
				checkState::End, checkEvent::Event, checkState::Inter, nullptr, nullptr
			),
			std::invalid_argument
		);
		CPPUNIT_ASSERT_EQUAL(checkState::Start, smup->state());

		// Check happy flow
		CPPUNIT_ASSERT(smup->event(checkEvent::Event, true));
		CPPUNIT_ASSERT_EQUAL(checkState::Inter, smup->state());

		// Invalid event returns false and keeps state
		CPPUNIT_ASSERT_EQUAL(false, smup->event(checkEvent::Event, true));
		CPPUNIT_ASSERT_EQUAL(checkState::Inter, smup->state());

		// Happy flow to end state
		CPPUNIT_ASSERT(smup->event(checkEvent::Quit, true));
		CPPUNIT_ASSERT_EQUAL(checkState::End, smup->state());

		// Don't leave end state
		CPPUNIT_ASSERT_EQUAL(false, smup->event(checkEvent::Event, true));
		CPPUNIT_ASSERT_EQUAL(checkState::End, smup->state());
		CPPUNIT_ASSERT_EQUAL(false, smup->event(checkEvent::Quit, true));
		CPPUNIT_ASSERT_EQUAL(checkState::End, smup->state());

		// Throw when adding transition in end state
		CPPUNIT_ASSERT_THROW(
			smup->transition(
				checkState::Inter, checkEvent::Event, checkState::End, nullptr, nullptr
			),
			std::logic_error
		);
	}

	void funccall()
	{

	}
