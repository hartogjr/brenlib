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

#include <bren/IOctxtWrapper.hpp>

namespace Bren {

	IOctxtWrapper::IOctxtWrapper()
	: Bren::Singleton<IOctxtWrapper>::Singleton()
	{
	}

	IOctxtWrapper::~IOctxtWrapper()
	{
		this->stop();
	}

	void IOctxtWrapper::runHere()
	{
		std::lock_guard<std::recursive_mutex> grd(mux_);
		this->stop();
		ioctxt_.run();
	}

	void IOctxtWrapper::runThreads(const uint16_t numThreads_i)
	{
		std::lock_guard<std::recursive_mutex> grd(mux_);
		this->stop();
		if (numThreads_i == 0) return;
		threads_.resize(numThreads_i);
		// Can't do this inside resize() unfortunately
		for (auto & t : threads_) t = std::thread([this]() { ioctxt_.run(); });
	}

	void IOctxtWrapper::stop()
	{
		std::lock_guard<std::recursive_mutex> grd(mux_);
		bool oldSWI = stopWhenIdle();

		if (!oldSWI) stopWhenIdle(true);
		if (!ioctxt_.stopped()) ioctxt_.stop();
		for (auto & t : threads_) t.join();
		threads_.clear();
		stopWhenIdle(oldSWI);
	}

	void IOctxtWrapper::stopWhenIdle(const bool stopWhenIdle_i)
	{
		std::lock_guard<std::recursive_mutex> grd(mux_);
		if (stopWhenIdle_i) {
			if (work_) {
				work_->reset();
				work_.reset();
			}
		} else {
			if (!work_) {
				work_.reset(
					new boost::asio::executor_work_guard<
						boost::asio::io_context::executor_type
					>(boost::asio::make_work_guard(ioctxt_))
				);
			}
		}
	}

} // Bren namespace
