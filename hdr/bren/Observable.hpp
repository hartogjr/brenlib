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

#pragma once

#include <mutex>
#include <list>
#include <bren/Observer.hpp>

namespace Bren {
	/** Keep track of observers that are observing us and notify them when
	 * the observed value changes. Everything is administered through weak
	 * pointers to prevent destruction deadlocks and shared pointer loops.
	 */
	template <class T>
	class Observable
	{
		private:
			// Mutex to prevent race conditions
			std::mutex mux_;

			// Set of observers
			std::list<std::weak_ptr<Observer<T>>> observers_;

			// Keep track of the actual value
			T value_;

		public:
			// Constructor to initialize internal value
			Observable(const T & value_i)
			: value_(value_i) {}

			// Destructor
			~Observable() {}

			/** Safety method that gets the weak shared pointer from the
			 * given observer to prevent segfaults when propagating changes
			 * to the observed value.
			 * @returns False when not actually registered, true when it
			 * is. */
			bool addObserver(Observer<T> * observer_i) {
				if (observer_i == nullptr) return false;
				return addObserver(observer_i->weak_from_this());
			}

			/** Add an observer to this observable to receive future value updates.
			 * @param observer_i Weak pointer to Observer that will be observing us.
			 * @returns True if @p observer_i is newly observing us, false if
			 * it was already there or if the given @p observer_i weak
			 * pointer has expired. */
			bool addObserver(std::weak_ptr<Observer<T>> observer_i) {
				// Parameter shared pointer
				auto psp = observer_i.lock();
				if (!psp) return false;

				std::lock_guard<std::mutex> lg(mux_);
				// Have to check for duplicates :-(
				for (auto i = observers_.begin(); i != observers_.end(); ) {
					// Iterator shared pointer
					auto isp = i->lock();
					if (!isp) {
						i = observers_.erase(i);
						continue;
					}
					// Do they point to the same object?
					if (isp.get() == psp.get()) return false;
					i++;
				}

				observers_.push_back(observer_i);
				return true;
			}

			// Retrieve the value
			T get() const { return value_; }

			/** Get the current list of observers. */
			const std::list<std::weak_ptr<Observer<T>>> & observers() const { return observers_; }

			/** Remove an observer to prevent future updates.
			 * @param observer_i Observer that is to be removed.
			 * @returns True if @p observer_i was removed, false if
			 * it was not present in the first place. */
			bool removeObserver(Observer<T> * observer_i) {
				if (observer_i == nullptr) return false;
				return removeObserver(observer_i->weak_from_this());
			}

			/** Remove an observer to prevent future updates.
			 * @param observer_i Observer that is to be removed.
			 * @returns True if @p observer_i was removed, false if
			 * it was not present in the first place. */
			bool removeObserver(std::weak_ptr<Observer<T>> observer_i) {
				auto psp = observer_i.lock();
				if (!psp) return false;

				std::lock_guard<std::mutex> lg(mux_);
				for (auto i = observers_.begin(); i != observers_.end(); ) {
					// Iterator shared pointer
					auto isp = i->lock();
					if (!isp) {
						i = observers_.erase(i);
						continue;
					}
					// Do they point to the same object?
					if (isp.get() == psp.get()) {
						i = observers_.erase(i);
						return true;
					}
					i++;
				}
				return false;
			}

			/** Set the value and broadcast it if it actually changed.
			 * @param value_i The new value to set.
			 * @returns True if value was changed and broadcasted to
			 * observers, false if value is unchanged. */
			bool set(const T & value_i) {
				std::unique_lock<std::mutex> lck(mux_);
				if (value_i == value_) return false;
				value_ = value_i;

				/** Make a deep copy of all the Observers To Be Notified so
				 * we can unlock the mutex for deadlock prevention. */
				auto otbn = observers_;
				lck.unlock();
				bool foundExpired = false;
				for (auto i = otbn.begin(); i != otbn.end(); i++) {
					// Shared Pointer to Observer
					auto spo = i->lock();
					if (!spo) {
						foundExpired = true;
						continue;
					}
					spo->observableChanged(value_);
				}

				if (foundExpired) {
					lck.lock();
					observers_.remove_if([](std::weak_ptr<Observer<T>> wp) { return wp.expired(); });
				}

				return true;
			}

	};

} // Bren namespace
