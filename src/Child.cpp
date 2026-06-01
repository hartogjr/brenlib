/* @author    Bren de Hartog <bren@dehartog.name>
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

#include <fstream>
#include <new>
#include <stdexcept>
#include <bren/Child.hpp>
#include <bren/Logger.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define BATCH 64

namespace fs = std::filesystem;

namespace Bren {

	bool Child::addEnv(const std::string & variable, const std::string & value)
	{
		bool ret = env_.contains(variable);
		env_[variable] = value;
		return ret;
	}

	void Child::capture(const bool capout, const bool caperr)
	{
		capStderr_ = caperr;
		capStdout_ = capout;
	}

	void Child::command(const fs::path & executable, const std::vector<std::string> & arguments)
	{
		FCET(!executable.empty(), std::invalid_argument, "Executable path can't be empty");
		fs::file_status stat = fs::status(executable);
		FCET(fs::exists(stat), std::invalid_argument, "Executable {:s} does not exist",
			executable.string());
		FCET(fs::is_regular_file(stat), std::invalid_argument,
			"Executable {:s} is not (a symbolic link to) a regular file", executable.string());

		cmd_ = executable;
		args_ = arguments;
	}

	uint8_t Child::run()
	{
		FCET(!cmd_.empty(), std::logic_error, "No executable command set yet");

		readEnv_();

		int pipefds[2] = {-1, -1};
		if (capStdout_ || capStderr_) {
			errno = 0;
			FCET(pipe(pipefds) == 0, std::runtime_error, "Failed to create pipe: {:s}",
				std::strerror(errno));
		}
		pid_t child = fork();
		if (child == 0) {
			// Inside child
			if (capStdout_ || capStderr_) {
				close(pipefds[0]);
				errno = 0;
				if (capStdout_) {
					FCET(dup2(pipefds[1], 1) != -1, std::runtime_error,
						"Failed duplicating pipe to stdout: {:s}", std::strerror(errno));
				}
				if (capStderr_) {
					FCET(dup2(pipefds[1], 2) != -1, std::runtime_error,
						"Failed duplicating pipe to stderr: {:s}", std::strerror(errno));
				}
			}

			char **args = new char *[args_.size() + 2];
			FCEA(args != nullptr, throw std::bad_alloc(), "Failed allocating memory for arguments");
			args[0] = new char[cmd_.string().size() + 1];
			FCEA(args[0] != nullptr, throw std::bad_alloc(),
				"Failed allocating memory for argument 0: {:s}", cmd_.native());
			std::strcpy(args[0], cmd_.c_str());
			args[args_.size() + 1] = nullptr;

			size_t pos = 1;
			for (const std::string & arg : args_) {
				args[pos] = new char[arg.size() + 1];
				FCEA(args[pos] != nullptr, throw std::bad_alloc(),
					"Failed allocating memory for argument {:d}: {:s}", pos, arg);
				std::strcpy(args[pos], arg.c_str());
				pos++;
			}

			char **env = new char *[env_.size() + 1];
			FCEA(env != nullptr, throw std::bad_alloc(),
				"Failed allocating memory for environment");
			env[env_.size()] = nullptr;

			pos = 0;
			for (const auto & pr : env_) {
				std::string envline = fmt::format("{:s}={:s}", pr.first, pr.second);
				env[pos] = new char[envline.size() + 1];
				FCEA(env[pos] != nullptr, throw std::bad_alloc(),
					"Failed allocating memory for environment variable {:s}={:s}",
					pr.first, pr.second);
				std::strcpy(env[pos], envline.c_str());
				pos++;
			}

			errno = 0;
			FCE(execve(args[0], args, env) != -1, "Execve call for {:s} failed: {:s}",
				args[0], std::strerror(errno));
			std::exit(1);
			// End of child, either way
		}
		// Inside parent

		// Do we have output to capture?
		if (capStdout_ || capStderr_) {
			close(pipefds[1]);

			output.clear();

			std::vector<char> data;
			size_t pos = 0;
			ssize_t num = 0;
			do {
				data.resize(pos + BATCH);
				errno = 0;
				num = read(pipefds[0], data.data() + pos, BATCH);
				if (num == -1) {
					FE("Failed reading output from child: {:s}", std::strerror(errno));
					break;
				}
				pos += num;
			} while (num > 0);
			data.resize(pos+1);
			data[pos] = '\0';
			output = data.data();

			close(pipefds[0]);
		}

		int status = 0;
		FCET(waitpid(child, &status, 0) == child, std::runtime_error,
			"Waiting for child did not return child PID?");
		FCET(WIFEXITED(status), std::runtime_error, "Child got terminated somehow?");
		return WEXITSTATUS(status);
	}

	void Child::readEnv_()
	{
		if (triedEnv_) return;
		triedEnv_ = true;

		size_t pos = 0;
		std::string envvar, val;
		std::ifstream profenv("/etc/profile.env");
		FCIA(profenv.good(), return, "Unable to open /etc/profile.env");

		for (std::string line; std::getline(profenv, line);) {
			if (line.empty()) continue;
			if (line[0] == '#') continue; // Comment

			if (line.size() < 10) continue; // Can't contain useful export command
			if (line.substr(0, 7) != "export ") continue; // Not an export command, skip

			pos = line.find('=', 7);
			if (pos == std::string::npos) continue; // No equal sign found

			envvar = line.substr(7, pos - 7);
			val = line.substr(pos + 2, line.size() - pos - 3);
			if (env_.contains(envvar)) continue;
			env_[envvar] = val;
		}

		profenv.close();
	}

} // Bren namespace
