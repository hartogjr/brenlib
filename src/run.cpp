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

#include <stdexcept>
#include <string.h>
#include <unistd.h>
#include <bren/run.hpp>

namespace Bren {

	uint8_t run(const std::string & command_i, const std::vector<std::string> & arguments_i)
	{
		if (!command_i.size()) throw std::logic_error("Command can't be empty");
		if (!arguments_i.size()) throw std::logic_error("At least one argument required as argv[0]");

		pid_t child = fork();
		char **argv;
		uint16_t i;
		int status;

		switch (child) {
			case -1:
				throw std::runtime_error("Failed to fork");

			case 0:
				// In the child
				argv = new char*[arguments_i.size() + 1];
				for (i = 0; i < arguments_i.size(); i++) {
					argv[i] = new char[arguments_i[i].size()+1];
					strcpy(argv[i], arguments_i[i].c_str());
				}
				argv[i] = NULL;
				execv(command_i.c_str(), argv);
				throw std::runtime_error("Failed execv(3)-ing the command");

			default:
				// Parent code below
				break;
		}

		pid_t ret = waitpid(child, &status, 0);
		if (ret != child) {
			throw std::runtime_error("Waitpid(2) returned incorrect PID");
		}
		if (!WIFEXITED(status)) {
			throw std::runtime_error("Child exited abnormally");
		}

		return WEXITSTATUS(status);
	}

} // Bren namespace
