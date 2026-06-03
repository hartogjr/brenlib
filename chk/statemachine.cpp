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
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <bren/StateMachine.hpp>

#define CHECKNAME StateMachineCheck

using std::placeholders::_1, std::placeholders::_2;

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

enum class checkState : uint8_t {
	Start,
	Inter,
	End
};

enum class checkEvent : uint8_t {
	Event,
	Quit
};

class CallReporter : public std::enable_shared_from_this<CallReporter> {

	private:
	struct Priv { explicit Priv() = default; };

	public:
	std::vector<std::pair<checkState, std::string>> called;

	// Constructor is only usable by this class
	CallReporter(Priv) {}

	static std::shared_ptr<CallReporter> create()
	{
		return std::make_shared<CallReporter>(Priv());
	}

	std::shared_ptr<CallReporter> getptr()
	{
		return shared_from_this();
	}

	bool callback(const checkState st, const std::string & stnam)
	{
		called.emplace_back(std::make_pair(st, stnam));
		return st != checkState::End;
	}

};

class CHECKNAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CHECKNAME);
	CPPUNIT_TEST(basics);
	CPPUNIT_TEST(funccall);
	CPPUNIT_TEST_SUITE_END();

	friend class Bren::StateMachine<checkState, checkEvent, CallReporter>;

	protected:
	/// State machine instance for easy setup and teardown
	std::unique_ptr<Bren::StateMachine<checkState, checkEvent, CallReporter>> smup;

	public:

	void setUp()
	{
		smup.reset(
			new Bren::StateMachine<checkState, checkEvent, CallReporter>(
				checkState::Start, checkState::End
			)
		);
		STATE_MACHINE_TRANSITION(
			(*smup), checkState::Start, checkEvent::Event, checkState::Inter, nullptr, nullptr);
		STATE_MACHINE_TRANSITION(
			(*smup), checkState::Start, checkEvent::Quit, checkState::End, nullptr, nullptr);
		STATE_MACHINE_TRANSITION(
			(*smup), checkState::Inter, checkEvent::Quit, checkState::End, nullptr, nullptr);
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
		auto cr = CallReporter::create();
		CPPUNIT_ASSERT_NO_THROW(smup->obj(cr));
		CPPUNIT_ASSERT_NO_THROW(smup->observe(std::bind(&CallReporter::callback, cr, _1, _2)));

		// Check that adding an observer already calls the callback with the current state
		CPPUNIT_ASSERT_EQUAL(1UL, cr->called.size());
		CPPUNIT_ASSERT_EQUAL(checkState::Start, cr->called.at(0).first);
		CPPUNIT_ASSERT_EQUAL(std::string("Start"), cr->called.at(0).second);

		// Actually transition
		CPPUNIT_ASSERT(smup->event(checkEvent::Event, true));
		CPPUNIT_ASSERT_EQUAL(2UL, cr->called.size());
		CPPUNIT_ASSERT_EQUAL(checkState::Inter, cr->called.at(1).first);
		CPPUNIT_ASSERT_EQUAL(std::string("Inter"), cr->called.at(1).second);
	}

}; // CHECKNAME class
