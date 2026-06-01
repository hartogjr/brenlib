#!/bin/bash
#
# Copyright (c) 2025, Bren de Hartog <bren@dehartog.name>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

namespace=""

function findLicenseFile() {
	if [ $# -ne 1 ]
	then
		echo "Please pass a directory to look for the license file" >&2
		echo ""
		return
	fi

	if [ -r "$1/LICENSE" ]
	then
		echo "$1/LICENSE"
		return
	fi

	if [ "$1" = "/" ]
	then
		echo ""
		return
	fi

	findLicenseFile "$(readlink -f $1/..)"
}

function writeBashLicense() {
	if [ $# -ne 1 ]
	then
		echo "Please specify the license file to format" >&2
		exit 1
	fi

	sed -E \
	-e "s/[0-9][0-9][0-9][0-9]/$(date +%Y)/g" \
	-e 's/^./# &/g' \
	-e 's/^$/#/g' "$1"
}

function writeCppLicense() {
	if [ $# -ne 1 ]
	then
		echo "Please specify the license file to format" >&2
		exit 1
	fi

	echo -e -n "/* "
	head -n 1 "$1" | sed -E -e "s/[0-9][0-9][0-9][0-9]/$(date +%Y)/g"
	tail -n +2 "$1" | sed -E \
		-e "s/[0-9][0-9][0-9][0-9]/$(date +%Y)/g" \
		-e 's/^./ * &/g' \
		-e 's/^$/ */g'
	echo " */"
}

function setns() {
	if [ -n "${namespace}" ]
	then
		# Already done
		return
	fi

	if [ $(find . -maxdepth 1 -name "*.cpp" -o -name "*.hpp" | wc -l) -gt 0 ]
	then
		namespace=$(awk '
		BEGIN {
			cnt["Bren"] = 0
		}

		/^namespace / {
			if ($2 in cnt) cnt[$2]++;
			else cnt[$2] = 1
		}

		END {
			winner = "Bren";
			wincount = 0;

			for (ns in cnt) {
				if (cnt[ns] > wincount) {
					winner = ns;
					wincount = cnt[ns];
				}
			}

			printf "%s\n", winner
		}' *.cpp *.h)
	else
		namespace="Bren"
	fi
}
