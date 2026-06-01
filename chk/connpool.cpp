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
POSSIBILITY OF SUCH DAMAGE.

vim:set ts=4 sw=4 noexpandtab: */

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <pqxx/pqxx>
#include <string>
#include <thread>
#include <Logger.h>
#include <Tracer.h>
#include <unistd.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ConnPool.h>

#define CHECKNAME ConnPoolCheck
#define TEMPLATEDB "host=pgdb user=postgres dbname=template1"
#define POSTGRESDB "host=pgdb user=postgres dbname=postgres"

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

std::mutex conmux;
std::mutex termmux;
std::condition_variable concdv, termcdv;
bool connected, stop;

class CHECKNAME : public CppUnit::TestFixture
{
		CPPUNIT_TEST_SUITE(CHECKNAME);
		CPPUNIT_TEST(connect);
		CPPUNIT_TEST(purge);
		CPPUNIT_TEST(multi);
		CPPUNIT_TEST_SUITE_END();

	private:
		std::shared_ptr<Bren::ConnPool<pqxx::nullconnection> > cp;
		std::unique_ptr<std::thread> t1, t2;

	public:
		uint64_t simpleThreadId(
                const std::thread::id & id_i = std::this_thread::get_id()
		) {
			std::stringstream ss;
			uint64_t stid;

			ss << id_i;
			ss >> stid;
			return stid;
        }

		void setUp() {
			CPPUNIT_ASSERT_EQUAL(false, (bool) cp);
			cp = std::make_shared<Bren::ConnPool<pqxx::nullconnection> >();
			CPPUNIT_ASSERT_EQUAL(true, (bool) cp);
			connected = false;
			stop = false;
		}

		void tearDown() {
			stop = true;

			// Make sure all threads terminate
			termcdv.notify_all();

			// Join threads
			if (t1) t1->join();

			if (t2) t2->join();

			cp.reset();
			CPPUNIT_ASSERT_EQUAL(false, (bool) cp);
		}

		void connThread() {
			auto dbc = cp->get(TEMPLATEDB);
			connected = true;
			concdv.notify_one();
			std::unique_lock<std::mutex> lck(termmux);
			termcdv.wait_for(
				lck,
				std::chrono::milliseconds(500),
				[] { return stop; }
			);
			CPPUNIT_ASSERT(stop);
		}

		void connect() {
			CPPUNIT_ASSERT_EQUAL((size_t) 0, cp->pool_a.size());
			CPPUNIT_ASSERT_EQUAL((size_t) 0, cp->pool_a.count(std::string(TEMPLATEDB)));
			auto dbc = cp->get(TEMPLATEDB);
			CPPUNIT_ASSERT(dbc.get() != nullptr);
			CPPUNIT_ASSERT_EQUAL(2L, dbc.use_count());
			CPPUNIT_ASSERT_EQUAL((size_t) 1, cp->pool_a.size());
			CPPUNIT_ASSERT_EQUAL((size_t) 1, cp->pool_a.count(std::string(TEMPLATEDB)));

			auto i = cp->pool_a.begin();
			CPPUNIT_ASSERT(i != cp->pool_a.end());
			CPPUNIT_ASSERT_EQUAL((size_t) 1, i->second.size());
			auto j = i->second.begin();
			CPPUNIT_ASSERT(j != i->second.end());
			CPPUNIT_ASSERT_EQUAL(dbc, j->second);
			CPPUNIT_ASSERT_EQUAL(simpleThreadId(), j->first);

			/** Check what happens when we call up a second connection from
			 * the same thread */
			{
				auto dbc2 = cp->get(TEMPLATEDB);
				CPPUNIT_ASSERT(dbc2.get() == dbc.get());
				CPPUNIT_ASSERT_EQUAL(3L, dbc.use_count());
				CPPUNIT_ASSERT_EQUAL(3L, dbc2.use_count());
				CPPUNIT_ASSERT_EQUAL((size_t) 1, cp->pool_a.size());
				CPPUNIT_ASSERT_EQUAL((size_t) 1, cp->pool_a.count(std::string(TEMPLATEDB)));

				CPPUNIT_ASSERT_EQUAL((size_t) 1, i->second.size());
				j = i->second.begin();
				CPPUNIT_ASSERT(j != i->second.end());
				CPPUNIT_ASSERT_EQUAL(dbc, j->second);
				CPPUNIT_ASSERT_EQUAL(simpleThreadId(), j->first);
			}
			/// Scope ends for second connection dbc2, check stats
			CPPUNIT_ASSERT(dbc.get() != nullptr);
			CPPUNIT_ASSERT_EQUAL(2L, dbc.use_count());
		}

		void purge() {
			CPPUNIT_ASSERT_EQUAL((size_t) 0, cp->pool_a.size());

			// Purge on empty connection list succeeds
			CPPUNIT_ASSERT_NO_THROW(cp->purge());

			{
				// Acquire connection 1
				auto dbc = cp->get(TEMPLATEDB);
				CPPUNIT_ASSERT_EQUAL((size_t) 1, cp->pool_a.size());
				auto i = cp->pool_a.find(std::string(TEMPLATEDB));
				CPPUNIT_ASSERT(i != cp->pool_a.end());
				CPPUNIT_ASSERT_EQUAL((size_t) 1, i->second.size());

				// Purge with active connection does nothing
				CPPUNIT_ASSERT_NO_THROW(cp->purge());
				CPPUNIT_ASSERT_EQUAL((size_t) 1, cp->pool_a.size());
				i = cp->pool_a.find(std::string(TEMPLATEDB));
				CPPUNIT_ASSERT(i != cp->pool_a.end());
				CPPUNIT_ASSERT_EQUAL((size_t) 1, i->second.size());
			}

			{
				// Acquire connection 2
				auto dbc2 = cp->get(POSTGRESDB);
				CPPUNIT_ASSERT_EQUAL((size_t) 2, cp->pool_a.size());
				auto i = cp->pool_a.find(std::string(POSTGRESDB));
				CPPUNIT_ASSERT(i != cp->pool_a.end());
				CPPUNIT_ASSERT_EQUAL((size_t) 1, i->second.size());

				// Connection 1 is kept
				i = cp->pool_a.find(std::string(TEMPLATEDB));
				CPPUNIT_ASSERT(i != cp->pool_a.end());
				CPPUNIT_ASSERT_EQUAL((size_t) 1, i->second.size());

				// Only purging busy conn 2 leaves other conn 1 alive
				CPPUNIT_ASSERT_NO_THROW(cp->purge(POSTGRESDB));
				i = cp->pool_a.find(std::string(TEMPLATEDB));
				CPPUNIT_ASSERT(i != cp->pool_a.end());
				CPPUNIT_ASSERT_EQUAL((size_t) 1, i->second.size());
			}

			// Purging connection 1 disconnects it ...
			CPPUNIT_ASSERT_NO_THROW(cp->purge(TEMPLATEDB));
			auto i = cp->pool_a.find(std::string(TEMPLATEDB));
			CPPUNIT_ASSERT(i == cp->pool_a.end());

			// and leaves other idle conn 2 intact
			i = cp->pool_a.find(std::string(POSTGRESDB));
			CPPUNIT_ASSERT(i != cp->pool_a.end());
			CPPUNIT_ASSERT_EQUAL((size_t) 1, i->second.size());

			// Purge with idle connection closes it
			CPPUNIT_ASSERT_NO_THROW(cp->purge());
			CPPUNIT_ASSERT_EQUAL((size_t) 0, cp->pool_a.size());
		}

		void multi() {
			uint64_t tid1, tid2;

			CPPUNIT_ASSERT_EQUAL((size_t) 0, cp->pool_a.size());

			auto dbc0 = cp->get(TEMPLATEDB);

			{
				CPPUNIT_ASSERT_EQUAL(false, connected);
				CPPUNIT_ASSERT_EQUAL(false, stop);
				std::unique_lock<std::mutex> lck(conmux);
				t1.reset(new std::thread(&CHECKNAME::connThread, this));
				tid1 = simpleThreadId(t1->get_id());

				// Wait until t1 is connected
				concdv.wait_for(
					lck,
					std::chrono::milliseconds(500),
					[] { return connected; }
				);
				CPPUNIT_ASSERT(connected);
			}

			CPPUNIT_ASSERT_EQUAL((size_t) 1, cp->pool_a.size());
			auto i = cp->pool_a.find(std::string(TEMPLATEDB));
			CPPUNIT_ASSERT(i != cp->pool_a.end());
			CPPUNIT_ASSERT_EQUAL((size_t) 2, i->second.size());
			bool found = false;

			for (auto & m : i->second) {
				CPPUNIT_ASSERT(
					m.first == simpleThreadId() ||
					m.first == tid1
				);

				if (m.first == tid1) found = true;
			}

			CPPUNIT_ASSERT_EQUAL(true, found);

			// Terminate and remove T1
			stop = true;
			termcdv.notify_one();
			t1->join();
			t1.reset();

			// Create separate thread that reuses the thread ID from t1, so
			// t2 has a different thread ID
			stop = false;
			t1.reset(new std::thread([]() {
				std::unique_lock<std::mutex> lck(termmux);
				termcdv.wait_for(
					lck,
					std::chrono::milliseconds(500),
					[] { return stop; }
				);
			}));

			connected = false;
			// New thread, same conn, reuse from t1
			{
				std::unique_lock<std::mutex> lck(conmux);
				t2.reset(new std::thread(&CHECKNAME::connThread, this));
				tid2 = simpleThreadId(t2->get_id());

				// Wait until t2 is connected
				concdv.wait_for(
					lck,
					std::chrono::milliseconds(500),
					[] { return connected; }
				);
				CPPUNIT_ASSERT(connected);
			}

			CPPUNIT_ASSERT(tid1 != tid2);
			CPPUNIT_ASSERT_EQUAL((size_t) 1, cp->pool_a.size());
			i = cp->pool_a.find(std::string(TEMPLATEDB));
			CPPUNIT_ASSERT(i != cp->pool_a.end());
			CPPUNIT_ASSERT_EQUAL((size_t) 2, i->second.size());
			found = false;
			bool bad = false;

			for (auto & m : i->second) {
				if (m.first == tid2) found = true;

				if (m.first == tid1) bad = true;

				CPPUNIT_ASSERT(
					m.first != tid1 && (
						m.first == simpleThreadId() ||
						m.first == tid2
					)
				);
			}

			CPPUNIT_ASSERT(found && !bad);
			stop = true;
			termcdv.notify_all();
		}

};
