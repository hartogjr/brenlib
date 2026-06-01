/** @author   Bren de Hartog <bren@dehartog.name>
 * @copyright Copyright (c) 2026, Bren de Hartog. All rights reserved.
 * @license   This project is licensed under the 3-clause BSD license:
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

#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <bren/Logger.hpp>

#define STATE_MACHINE_TRANSITION(obj, from, event, to, onTrans, inState) \
	obj.name(from, #from); \
	obj.name(event, #event); \
	obj.name(to, #to); \
	obj.transition(from, event, to, onTrans, inState)

namespace Bren
{
	/** StateMachine to track transitions and call appropriate code.
	 * It is modelled after an Activity-On-Arc (AOA) finite state machine.
	 * From a graph theory point of view, a StateMachine object holds a labelled directed
	 * multigraph permitting loops.
	 * @tparam STATE The Enum class type of the states the FSM can be in.
	 * @tparam EVENT Enum class type of all events that can be triggered.
	 * @tparam HANDLER Class that implements all called methods on transitions. */
	template <class STATE, class EVENT, class HANDLER>
	class StateMachine
	{
		public:
		/** Type definition of transition function */
		typedef std::function<bool(HANDLER *)> transfunc;

		/** Type definition of state function */
		typedef std::function<void(HANDLER *)> statefunc;

		protected:
		/// Flag to keep track of whether we're active in handling an event
		bool active_;

		/// Final end state in which resources should be released
		const STATE end_;

		/// Hash map of event names
		std::unordered_map<EVENT, std::string> events_;

		/// Mutex to prevent race conditions
		mutable std::mutex mux_;

		/// Hash map of state names
		std::unordered_map<STATE, std::string> states_;

		/// Actual object to pass to transition calls
		std::shared_ptr<HANDLER> obj_;

		/// Start state
		const STATE start_;

		/// Current state
		STATE state_;

		/// Struct for combining a destination state and functions to be called
		struct transTarget {
			STATE target;
			transfunc onTrans;
			statefunc inState;
		};

		/// List of allowed transitions
		std::unordered_map<STATE, std::unordered_map<EVENT, transTarget> > trans_;

		/** Lookup the name of an event.
		 * @param event_i Event to lookup the name for
		 * @returns Name of a event if found, the state number (as string) if not. */
		std::string name_(const EVENT event_i) const {
			const auto & it = events_.find(event_i);
			if (it == events_.end()) {
				return fmt::format(fmt::runtime("{:s}"), fmt::underlying(event_i));
			}
			return it->second;
		}

		/** Lookup the name of a state.
		 * @param state_i State to lookup the name for
		 * @returns Name of a state if found, the state number (as string) if not. */
		std::string name_(const STATE state_i) const {
			const auto & it = states_.find(state_i);
			if (it == states_.end()) {
				return fmt::format(fmt::runtime("{:s}"), fmt::underlying(state_i));
			}
			return it->second;
		}

		/** Return the short name of the given string, stripping any namespaces. */
		std::string shorten_(const std::string & name_i) {
			size_t pos = name_i.rfind(":");
			if (pos == std::string::npos) return name_i;
			return name_i.substr(pos+1);
		}

		/** Actually transition to another state.
		 * This method is called asynchronously.
		 * @param from_i Current state from where transition takes place
		 * @param event_i The event that triggered the transition
		 * @param to_i The destination state for the transition
		 * @param onTrans_i The function to call on transitioning
		 * @param inState_i Function to call after successful transition */
		void transition_(
			const STATE from_i,
			const EVENT event_i,
			const STATE to_i,
			transfunc onTrans_i,
			statefunc inState_i
		) {
			std::unique_lock<std::mutex> lck(mux_);
			active_ = true;
			try {
				FD("Initiating state transition on event {:s} from state {:s} to {:s}",
					name_(event_i), name_(from_i), name_(to_i));
				lck.unlock();
				if (onTrans_i != nullptr) {
					if (onTrans_i(obj_.get())) {
						FI("onTransition function on event {:s} from state {:s} to "
							"{:s} completed!!", name_(event_i), name_(from_i), name_(to_i));
						lck.lock();
						state_ = to_i;
					} else {
						FI("onTransition function on event {event:s} from state {from:s} to {to:s} "
							"failed, staying at {from:s}", fmt::arg("event", name_(event_i)),
							fmt::arg("from", name_(from_i)), fmt::arg("to", name_(to_i)));
						lck.lock();
						state_ = from_i;
					}
				} else {
					FD("No onTransition function, so changing state from {:s} to {:s} on {:s}",
						name_(from_i), name_(to_i), name_(event_i));
					lck.lock();
					state_ = to_i;
				}
				lck.unlock();

				if (inState_i != nullptr) {
					FD("Executing inState function after transition from {:s} to {:s} on {:s}",
						name_(from_i), name_(to_i), name_(event_i));
					inState_i(obj_.get());
				}

			} catch (const std::exception & se) {
				FW("State transition function threw an exception: {:s}", se.what());
			} catch (...) {
				FW("State transition function threw an unknown exception");
			}
			lck.lock();
			if (state_ == end_) obj_.reset();
			active_ = false;
			lck.unlock();
		}

		public:
		/** Constructor.
		 * @param start_i Start state to use as initial state after construction.
		 * @param end_i End state in which the object should be released. No more transitions will
		 * be possible once this state is reached. */
		StateMachine(const STATE start_i, const STATE end_i)
		: active_(false), end_(end_i), start_(start_i), state_(start_i)
		{ }

		/** Check whether the object is active, i.e., handling an event.
		 * @returns True if an event is being handled, false if not. */
		bool active() const { return active_; }

		/** Write a <a href="https://graphviz.org/">Graphviz Dot</a> graph of the internal
		 * state machine.
		 * @returns String that can be written to a file or fed as stdin to the Dot program to
		 * generate a directed graph of the state machine. */
		std::string digraph() const {
			std::string dg = "digraph {\n";
			dg += ("\t\"__start__\" [style=\"filled\", label=\"\", fillcolor=\"black\", ");
			dg += ("shape=\"circle\", fixedsize=true, width=0.2];\n");
			dg += fmt::format("\t\"{:s}\" [peripheries=2];\n", name_(end_));

			dg += fmt::format("\t\"__start__\" -> \"{:s}\";\n", name_(start_));

			for (auto const & trs : trans_) {
				for (auto const & tgt : trs.second) {
					dg += fmt::format("\t\"{:s}\" -> \"{:s}\" [ label=\"{:s}\" ];\n",
						name_(trs.first), name_(tgt.second.target), name_(tgt.first));
				}
			}

			dg += "}\n";
			return dg;
		}

		/** Event trigger for an actual transition.
		 * @param event_i Event that triggered.
		 * @param sync_i True if call should wait for transition to finish, false if not.
		 * @returns True if transition is allowed and executed, false if a current transition is in
		 * progress or no transition is possible on this event from the current state. */
		bool event(const EVENT event_i, bool sync_i = false) {
			std::unique_lock<std::mutex> lck(mux_);

			FCIR(!active(), false, "Event handling still in progress");
			FCER(state_ != end_, false, "State machine already in end state {:s}", name_(end_));
			FCER(obj_, false, "Shared pointer to object for function calling not set");

			auto const src = trans_.find(state_);
			FCWR(src != trans_.end(), false,
				"Unable to find state {:s} as source for transition", name_(state_));
			auto const dst = src->second.find(event_i);
			FCWR(dst != src->second.end(), false, "No event {:s} registered for transition "
				"from state {:s}", name_(event_i), name_(state_));

			// Then execute transition synchronously or asynchronously
			if (sync_i) {
				lck.unlock();
				transition_(state_, event_i, dst->second.target, dst->second.onTrans,
					dst->second.inState);
			} else {
				std::thread(&StateMachine::transition_, this, state_, event_i, dst->second.target,
					dst->second.onTrans, dst->second.inState).detach();
			}

			return true;
		}

		/** Register a name for an event.
		 * This method is called automatically by the STATE_MACHINE_TRANSITION() macro.
		 * When a name for an event has already been registered, this method will overwrite it with
		 * the next registration.
		 * @param event_i Event enum value
		 * @param name_i Name to store for event
		 * @throws std::invalid_argument if the event name was already used for another event. */
		void name(const EVENT event_i, const std::string & name_i) {
			std::string shrt = shorten_(name_i);
			std::lock_guard<std::mutex> lck(mux_);
			for (const auto & keyval : events_) {
				if (keyval.first == event_i) continue;
				FCET(name_i != keyval.second, std::invalid_argument,
					"Event name \"{:s}\" was already used for another event", shrt);
			}
			events_[event_i] = shrt;
		}

		/** Register a name for a state.
		 * This method is called automatically by the STATE_MACHINE_TRANSITION() macro.
		 * When a name for a state has already been registered, this method will overwrite it with
		 * the next registration.
		 * @param state_i State enum value
		 * @param name_i Name to store for state
		 * @throws std::invalid_argument if the state name was already used for another state. */
		void name(const STATE state_i, const std::string & name_i) {
			std::string shrt = shorten_(name_i);
			std::lock_guard<std::mutex> lck(mux_);
			for (const auto & keyval : states_) {
				if (keyval.first == state_i) continue;
				FCET(name_i != keyval.second, std::invalid_argument,
					"State name \"{:s}\" was already used for another state", shrt);
			}
			states_[state_i] = shrt;
		}

		/** Get the name of the given event as a string.
		 * @param event_i Event enum to return name for. */
		std::string name(const EVENT event_i) const {
			std::lock_guard<std::mutex> lck(mux_);
			return name_(event_i);
		}

		/** Get the name of the given state as a string.
		 * @param state_i State enum to return name for. */
		std::string name(const STATE state_i) const {
			std::lock_guard<std::mutex> lck(mux_);
			return name_(state_i);
		}

		/** NoOp method to allow transitions without calling a HANDLER method or having
		 * to write one. Have to add transistion with the following as function:
		 * decltype(stateMachineObject)::noop */
		static
		bool noop(HANDLER * obj_i) {
			UNUSED(obj_i);
			return true;
		}

		/** Set the object to use for function calling on transitions.
		 * @param obj_i Shared pointer to object */
		void obj(std::shared_ptr<HANDLER> obj_i)
		{
			obj_ = obj_i;
		}

		/** Get current state.
		 * @returns state the state machine is currently in. */
		STATE state() const { return state_; }

		/** Add a possible transition.
		 * @param from_i State from which transition is possible.
		 * @param event_i Event that will trigger this transition.
		 * @param to_i State to which transition is to take place.
		 * @param onTrans_i Function to call during transition, can return false to stop transition
		 * @param inState_i Function to call just after state transition
		 * @throws std::invalid_argument when @p from_i is the end state, when @p from_i and @p
		 * to_i are equal or when the given @p from_i and @p event_i combination was already
		 * registered.
		 * @throws std::logic_error when state machine is in end state.
		 * @throws std::runtime_error when a transition is active. */
		void transition(
			const STATE from_i, const EVENT event_i, const STATE to_i, transfunc onTrans_i,
			statefunc inState_i
		) {
			std::lock_guard<std::mutex> lck(mux_);
			FCET(!active(), std::runtime_error, "Unable to add transition when transition is in "
				"progress");
			FCET(from_i != end_, std::invalid_argument, "Can't transition away from "
				"end state {:s}", name_(end_));
			FCET(state_ != end_, std::logic_error, "Not adding transition when already in end ",
				"state {:s}", name_(end_));

			auto & eventMap = trans_[from_i];
			auto tgtit = eventMap.find(event_i);
			FCET(tgtit == eventMap.end(), std::invalid_argument,
				"Event {:s} from state {:s} already registered", name_(event_i), name_(from_i));
			eventMap[event_i] = { to_i, onTrans_i, inState_i };
			FD("Stored transition on event {:s} from state {:s} to state {:s}", name_(event_i),
				name_(from_i), name_(to_i));
		}

	};

} // Bren namespace
