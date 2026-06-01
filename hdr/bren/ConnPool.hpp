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

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <signal.h>

/// Forward checkclass declaration for friendships
class ConnPoolCheck;

namespace Bren {

	/** Database connection pool that manages all DB connections.
	 */
	template <class T>
	class ConnPool
	{
			/// Check class can look inside data structures
			friend class ::ConnPoolCheck;

		private:
			/// Copy constructor
			ConnPool(const ConnPool & obj_i) = delete;

			/// Assignment constructor
			ConnPool & operator=(const ConnPool & obj_i) = delete;

			/** Type definition for a map containing thread IDs to shared
			 * pointers with DB connections. */
			typedef std::map <
			uint64_t,
			std::shared_ptr<T>
			> connmap_t;

			/** Type definition for internal connection pool administration.
			 * The map has a string as key, containing the DB
			 * connection string. Only a literal string comparison is matched,
			 * permutations of settings are not taken into account. */
			typedef std::map <
			const std::string,
				  connmap_t
				  > pool_t;

			/// Internal mutex to control pool access
			std::mutex mux_a;

			/// Actual connection pool
			pool_t pool_a;

			/** Actively close idle connections within a specific pool.
			 * @param cm_i Connection map to purge of idle connections
			 * @returns True when specified map is empty after purge,
			 * false if not. */
			bool purge_(connmap_t & cm_i)
			{
				std::set<uint64_t> idles;

				for (auto & i : cm_i) {
					if (i.second.use_count() == 1) idles.insert(i.first);
				}
				for (auto & i : idles) cm_i.erase(i);
				return cm_i.size() == 0;
			}

		public:
			/// Constructor to ignore signals
			inline ConnPool() {
				// Ignore SIGPIPE, otherwise it terminates our application when a
				// connection can't be established or is broken
				signal(SIGPIPE, SIG_IGN);
			}

			/// Destructor to purge connections
			inline ~ConnPool() { purge(); }

			/** Shorthand type definition for DataBaseConnection. */
			typedef std::shared_ptr<T> dbc_t;

			/** Get a database connection using @p params_i as connection
			 * string.
			 * @param params_i A DB connection string. Each connection
			 * string comes with its own connection pool.
			 * @throws A runtime exception when the connection setup failed.
			 * @returns A DB connection inside a std::shared_ptr */
			dbc_t get(const std::string & params_i) {
				dbc_t con;
				std::stringstream ss;
				uint64_t stid;

				ss << std::this_thread::get_id();
				ss >> stid;

				std::lock_guard<std::mutex> lck(mux_a);

				auto i = pool_a.find(params_i);

				if (i == pool_a.end()) {
					/** Create new connection. This throws a
					 * pqxx::broken_connection exception when the connect fails.
					 * This is ok to be propagated to the caller. */
					con.reset(new T(params_i));
					connmap_t cm;
					cm[stid] = con;
					pool_a[params_i] = cm;
					return con;
				}

				// Thread is already using connection
				auto j = i->second.find(stid);

				if (j != i->second.end()) return j->second;

				// Thread has not used a connection recently, find free one
				for (j = i->second.begin(); j != i->second.end(); j++) {
					if (j->second.use_count() == 1) {
						// Connection is free
						con = j->second;
						i->second.erase(j->first);
						i->second[stid] = con;
						return con;
					}
				}

				// No free one found, create new
				con.reset(new T(params_i));
				i->second[stid] = con;

				return con;
			}

			/** Get a database connection using @p params_i as connection
			 * string.
			 * @param params_i A DB connection string. Each connection
			 * string comes with its own connection pool.
			 * @throws A runtime exception when the connection setup failed.
			 * @returns A DB connection inside a std::shared_ptr */
			inline dbc_t get(const char *params_i) {
				return get(std::string(params_i));
			}

			/** Actively close all currently idle connections in the pool.
			 * @param params_i Unique connection string identifying which pool
			 * to purge. Can be the empty string (also the default) to purge
			 * all pools of idle connections. */
			inline void purge(const std::string & params_i = "")
			{
				std::set<std::string> toErase;
				std::lock_guard<std::mutex> lck(mux_a);

				if (params_i == "") {
					for (auto & i : pool_a) {
						if (purge_(i.second)) toErase.insert(i.first);
					}
					for (auto & i : toErase) pool_a.erase(i);
					return;
				}

				auto i = pool_a.find(params_i);
				if (i == pool_a.end()) return;
				if (purge_(i->second)) pool_a.erase(i);
			}

	};

} // Bren namespace
