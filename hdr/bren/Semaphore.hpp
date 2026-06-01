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

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <stdexcept>

namespace Bren {

	class Semaphore
	{
		private:
		/// Internal condition variable
		std::condition_variable convar_a;

		/// Internal counter
		uint16_t count_a;

		/// Flag to indicate destruction
		std::atomic<bool> destructing_a;

		/// Internal mutex
		std::mutex mux_a;

		public:
		/// Constructor
		Semaphore(uint16_t count_i = 0);

		/// Destructor
		~Semaphore();

		/** Return the current internal semaphore counter.
		 * @returns The internal counter. */
		uint16_t count();

		/** Decrease the internal semaphore counter, blocking if count is zero.
		 */
		void dec();

		/** Increase the internal semaphore counter and notify a waiting thread,
		 * if any. */
		void inc();

		/** Try to decrease the internal semaphore counter, but do not block
		 * if it's count is currently zero.
		 * @returns True if counter was decreased, false if not (and thus would
		 * have blocked otherwise). */
		bool try_dec();

	};

} // Bren namespace
