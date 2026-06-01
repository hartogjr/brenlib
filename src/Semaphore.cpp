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

#include <bren/Semaphore.hpp>

namespace Bren {

	Semaphore::Semaphore(uint16_t count_i)
	: count_a(count_i), destructing_a(false)
	{ }

	Semaphore::~Semaphore()
	{
		destructing_a = true;
		convar_a.notify_all();
	}

	uint16_t Semaphore::count()
	{
		if (destructing_a) {
			throw std::logic_error("Semaphore is being destroyed while checking counter");
		}

		std::unique_lock<std::mutex> lck(mux_a);
		return count_a;
	}

	void Semaphore::dec()
	{
		if (destructing_a) {
			throw std::logic_error("Semaphore is being destroyed while decreasing counter");
		}

		std::unique_lock<std::mutex> lck(mux_a);

		while (destructing_a == false && count_a == 0) {
			convar_a.wait(lck);
		}

		if (destructing_a) {
			throw std::logic_error("Semaphore is being destroyed while decreasing counter");
		}
		count_a--;
	}

	void Semaphore::inc()
	{
		if (destructing_a) {
			throw std::logic_error("Semaphore is being destroyed while increasing counter");
		}

		std::unique_lock<std::mutex> lck(mux_a);
		count_a++;
		convar_a.notify_one();
	}

	bool Semaphore::try_dec()
	{
		if (destructing_a) {
			throw std::logic_error("Semaphore is being destroyed while decreasing counter");
		}

		std::unique_lock<std::mutex> lck(mux_a);
		if (count_a == 0) return false;
		count_a--;
		return true;
	}

} // Bren namespace
