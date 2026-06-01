#!/bin/bash

# BSD 3-Clause License
#
# Copyright (c) 2025, Bren de Hartog <bren@dehartog.name>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# vim:set ts=4 sw=4 noet tw=120:

if [ $# -ne 1 ]
then
	echo "Please provide a classname to create a check for"
	exit 1
fi

base=$(readlink -f $(dirname ${BASH_SOURCE[0]})/..)
source $base/bin/lib.sh

class=$(echo $1 | tr '[:upper:]' '[:lower:]')

if [ -r "${class}.cpp" ]; then
	echo "${class}.cpp already exists! Not overwriting it."
	exit 1
fi

lf="$(findLicenseFile ${PWD})"
if [ -z "$lf" ]
then
	echo "No license file found in ${PWD} or any parent directories."
	exit 1
fi

writeCppLicense "$lf" > ${class}.cpp

cat >> ${class}.cpp << EOF
 *
 * vim:set ts=4 sw=4 noet: */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <$1.h>

#define CHECKNAME ${class}Check

class CHECKNAME;

CPPUNIT_TEST_SUITE_REGISTRATION(CHECKNAME);

class CHECKNAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CHECKNAME);
	CPPUNIT_TEST(firstTest);
	CPPUNIT_TEST_SUITE_END();

	public:

	CHECKNAME()
	{ }

	void setUp() {
	}

	void tearDown() {
	}

	void firstTest() {

	}

};

#undef CHECKNAME
EOF
