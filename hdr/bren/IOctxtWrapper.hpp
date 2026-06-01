/* Copyright (c) 2025, Bren de Hartog <bren@dehartog.name>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <boost/asio/io_context.hpp>
#include <bren/Singleton.hpp>

namespace Bren {

	class IOctxtWrapper : public Bren::Singleton<IOctxtWrapper>
	{
		/// Singleton template as friend for construction
		friend class Bren::Singleton<IOctxtWrapper>;

		private:
		/// Default constructor
		IOctxtWrapper();

		/// Copy constructor
		IOctxtWrapper(const IOctxtWrapper & obj_i) = delete;

		/// Assignment constructor
		IOctxtWrapper & operator=(const IOctxtWrapper & obj_i) = delete;

		/// Destructor
		~IOctxtWrapper();

		protected:
		/// Actual Boost I/O context object
		boost::asio::io_context ioctxt_;

		/// Mutex to prevent race conditions
		std::recursive_mutex mux_;

		/// Threads to run I/O context in
		std::vector<std::thread> threads_;

		/// Work guard object to keep io context running
		std::unique_ptr<
			boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
		> work_;

		public:
		/** Run the I/O context in place, blocking the calling thread. */
		void runHere();

		/** Run the I/O context in separate threads.
		 * @param numThreads_i Number of simultaneous threads to run I/O
		 * context in, default 1. Passing 0 as parameter calls stop(). */
		void runThreads(const uint16_t numThreads_i = 1);

		/** Check whether the I/O context would/will stop when it is idle.
		 * @returns True if it would/will stop, false if not. */
		inline bool stopWhenIdle() { return !work_; }

		/** Define whether the I/O context should/will stop running in case
		 * there is no more work available.
		 * @param stopWhenIdle_i True if it should stop running when idle,
		 * false if it should keep running forever. */
		void stopWhenIdle(const bool stopWhenIdle_i);

		/** Stop the I/O context.
		 * Also terminates the threads if necessary. */
		void stop();

		/** Retrieve the actual io_context object for use in other objects.
		 * @returns reference to internal io_context object. */
		inline boost::asio::io_context & context() { return this->ioctxt_; }

	};

} // Bren namespace
