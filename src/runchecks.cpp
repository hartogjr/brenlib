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

#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/program_options.hpp>
#include <bren/Logger.hpp>
#include <bren/runchecks.hpp>
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

namespace po = boost::program_options;

namespace Bren {

	/** Runs the check application.
	 * @returns 0 on success, 1 if a check failed. */
	int runchecks(int argc_i, char *argv_i[], const std::string & appname_i)
	{
		// Variables needed for option parsing
		po::options_description            desc(appname_i);
		po::positional_options_description pod;
		po::variables_map                  vm;
		std::vector<std::string>           filters;

		// Define and parse options
		// clang-format off
		desc.add_options()
		("help,h", "Show this helpful message")
		("list,l", "List all unitchecks")
		("filter,f", po::value< std::vector<std::string> >(&filters)->multitoken(),
			"Filter unitchecks to execute (also positional arguments)");
		// clang-fornat on
		pod.add("filter", -1);
		po::store(
			po::command_line_parser(argc_i, argv_i).options(desc).positional(pod).run(),
			vm
		);
		po::notify(vm);

		// If help is requested, deal with that first.
		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 1;
		}

		// Otherwise, initialize all checks
		CppUnit::TextUi::TestRunner runner;
		bool                        retval = false;

		// Set up test suite
		CppUnit::TestFactoryRegistry & registry = CppUnit::TestFactoryRegistry::getRegistry();
		CppUnit::Test * suite = registry.makeTest();
		std::unordered_map<std::string, CppUnit::Test *> tests;

		if (vm.count("list") || vm.count("filter")) {
			// Make a map from sanitized suite name to actual testsuite pointer.
			std::string     name, last;
			CppUnit::Test * tstptr = nullptr;
			for (int i = 0; i < suite->getChildTestCount(); i++) {
				tstptr = suite->getChildTestAt(i);
				name = tstptr->getName();
				if (name.size() >= 7) {
					last = name.substr(name.size() - 5);
					std::transform(last.begin(), last.end(), last.begin(), ::tolower);
					if (last == "check") name.resize(name.size() - 5);
				}
				tests[name] = tstptr;
			}

			if (vm.count("list")) {
				std::vector<std::string> testnames;
				for (auto const & item : tests) testnames.push_back(item.first);
				std::ranges::sort(testnames);
				for (auto const & tstname : testnames) {
					std::cout << tstname << std::endl;
				}
				return 1;
			}

			// We are filtering. Create a vector of lowercase filters from the command line.
			std::vector<std::string> lowfilters;
			std::string lowered;
			for (const std::string & item : filters) {
				lowered.erase();
				std::transform(item.begin(), item.end(), std::back_inserter(lowered), ::tolower);
				lowfilters.push_back(lowered);
			}

			for (auto & item : tests) {
				name = item.first;
				std::ranges::transform(name.begin(), name.end(), name.begin(), ::tolower);
				for (auto const & fltr : lowfilters) {
					if (name.find(fltr) != std::string::npos) {
						runner.addTest(item.second);
					}
				}
			}
		} else {
			runner.addTest(suite);
		}

		// Actually run the tests
		retval = runner.run();

		// Convert the C-style true (1) to Shell-style true (0) and v.v.
		return retval ? 0 : 1;
	}

} // Bren namespace
