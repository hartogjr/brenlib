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

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

namespace Bren {

	/** Fork a child with various settings, optionally capture output and wait for termination. */
	class Child
	{
		protected:
		/// List of arguments
		std::vector<std::string> args_;

		/// Should stderr be captured?
		bool capStderr_ = false;

		/// Should stdout be captured?
		bool capStdout_ = false;

		/// Command to execute
		fs::path cmd_;

		/// Environment to pass to child
		std::unordered_map<std::string, std::string> env_;

		/// Flag to keep track of whether we tried reading the environment from /etc/profile.env
		bool triedEnv_ = false;

		/// Read the environment from /etc/profile.env into env_
		void readEnv_();

		public:
		/// Output string
		std::string output;

		/// Constructor
		Child() = default;

		/// Destructor
		~Child() = default;

		/** Add an environment variable setting.
		 * @param variable Environment variable to set, uppercase is recommended
		 * @param value Value to set the environment variable to
		 * @returns True if an existing variable was overwritten, false if it was added. */
		bool addEnv(const std::string & variable, const std::string & value);

		/** Capture stdout and optionally stderr yes or no.
		 * Capturing is done to a single output string, there is no way to know which output came
		 * from which stream.
		 * @param capout Whether to capture stdout from the child process
		 * @param caperr Whether to capture stderr from the child process */
		void capture(const bool capout = false, const bool caperr = false);

		/** Specify the command as an executable with its arguments.
		 * The zeroth argument is automatically set to the executable, so should not be specified.
		 * @param executable Filename of executable or script with shebang.
		 * @param arguments optional arguments for the executable, excluding the zeroth argument. */
		void command(const fs::path & executable, const std::vector<std::string> & arguments = {});

		/** Actually run the child, passing it the given arguments and environment and capturing
		 * output if so desired. This method can be called multiple consecutive times to execute the
		 * same command more than once.
		 * @returns exit status of child */
		uint8_t run();
	};

} // Bren namespace
