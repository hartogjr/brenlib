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

#include <cstdint>
#include <stdexcept>
#include <string>
#include <errno.h>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <bren/Application.hpp>
#include <bren/IOctxtWrapper.hpp>
#include <bren/Logger.hpp>

#define STR(s) XSTR(s)
#define XSTR(s) #s

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
	int retval = 0;
	Bren::Logger *logger = nullptr;
	Bren::Application *app = nullptr;
	Bren::IOctxtWrapper * icw = nullptr;
	size_t strp = strlen(STR(GITREPOROOT))+1;

	try {
		po::options_description desc("This is a template daemon.\nThe default is to log to syslog using facility LOG_DAEMON. Command-line options:");

		desc.add_options()
			("help,h", "Show this help message")
			("debug,d", "Activate console logging to standard error instead of to syslog, implies -f")
			("foreground,f", "Do not daemonize, keep running in the foreground.")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << std::endl;
			throw 0;
		}

		logger = Bren::Logger::instance();

		if (vm.count("debug")) {
			logger->stderror(strp);
			LD("Logging to standard error with strip length %d", strp);
		} else {
			logger->syslog("brentpld", LOG_DAEMON, strp);
			LD("Logging to syslog with strip length %d", strp);
		}
		if (!vm.count("debug") && !vm.count("foreground")) {
			LD("No debugging or foreground operation requested, daemonizing.");
			errno = 0;
			LCE(daemon(0, 0) == 0, "Failed to daemonize: %s", strerror(errno));
			errno = 0;
		}

		icw = Bren::IOctxtWrapper::instance();
		app = Bren::Application::instance();

		// Here comes the core code ;-)
		icw->stopWhenIdle(false);
		if (!app->init(&(icw->context()))) throw 1;
		icw->runHere();

	} catch(const int & i) {
		retval = i;
	} catch(std::exception & se) {
		LE("General exception: %s", se.what());
		retval = 1;
	}

	if (app != nullptr) app->stop_listening();

	Bren::IOctxtWrapper::close();
	icw = nullptr;

	Bren::Singleton<Bren::Application>::close();
	app = nullptr;

	Bren::Logger::close();
	logger = nullptr;

	return retval;
}
