#!/bin/bash
# Copyright (c) 2025 Bren de Hartog <bren@dehartog.name>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
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
#
# vim:set ts=4 sw=4 noet tw=120:

# Add this script to the bin directory of your repository. Add the
# following lines to your CMakeLists.txt (example currency):
# add_custom_command (
# 	OUTPUT currency.h
# 	COMMAND ${CMAKE_SOURCE_DIR}/bin/tbl.sh
# 	-n Bren -o ${CMAKE_CURRENT_BINARY_DIR}
# 	${CMAKE_CURRENT_SOURCE_DIR}/currency.tbl
# 	MAIN_DEPENDENCY currency.tbl
# 	DEPENDS ${CMAKE_SOURCE_DIR}/bin/tbl.sh
# )
# And for forced generation of file:
# add_custom_target (GenFiles ALL DEPENDS currency.h)

function die() {
	echo $*
	exit 1
}

nsbegin=""
nsend=""
nsprefix=""
infile=""
outdir="."

while getopts ":hn:o:" opt
do
	case ${opt} in
		h)
			echo -n "Usage: $(basename \"$0\") "
			echo "[-h] [-n namespace] [-o outputdirectory] <inputfile>"
			echo
			echo "This script writes source code files .h .cpp using <inputfile>"
			echo "as basename and table input. Options:"
			echo
			echo "-h Print this help"
			echo "-n Set the namespace to be used when generating the source files"
			echo "-o Set the output directory to write source files to."
			echo "   Default is current directory."
			exit 1
			;;

		n)
			nsbegin="
namespace $OPTARG {
"
			nsend="
} // namespace $OPTARG"
			nsprefix="::$OPTARG::"
			;;

		o)
			export outdir="$OPTARG"
			;;

		\?)
			die "Invalid option: $OPTARG"
			;;

		:)
			die "Option $OPTARG requires an argument but doesn't have one"
			;;
	esac
done

[ $OPTIND -eq $# ] || die "Please specify at least the input file as a parameter (use -h for help)"
infile="${!OPTIND}"
[ -r "$infile" ] || die "Unable to read $infile"

entries=$[ $(wc -l $infile | cut -d' ' -f1) - 2 ]

if [ $entries -lt 256 ]
then
	sizetype="uint8_t"
else
	sizetype="uint16_t"
fi

export base=$( basename "$infile" .tbl )

cat > "${outdir}/${base}.h" <<EOF
// vim:set ts=4 sw=4 noet tw=120:

#pragma once

#include <cstdint>
#include <iostream>
$nsbegin
enum class ${base}_e : ${sizetype}
{
EOF

sed -r -e 's/([^@]+)@.*$/	\1,/g' < ${infile} >> "${outdir}/${base}.h"

cat >> "${outdir}/${base}.h" <<EOF
};

// Convert enum to Description
inline const char *${base}_d(const ${base}_e & ev_i)
{
	switch (ev_i) {
EOF
awk -F@ \
'{ print "\t\tcase '${base}_e::'" $1 ": return \"" $2 "\";" }' "${infile}" >> "${outdir}/${base}.h"

cat >> "${outdir}/${base}.h" <<EOF
		default: return nullptr;
	}
}

// Convert enum to basic String
inline const char *${base}_s(const ${base}_e & ev_i)
{
	switch (ev_i) {
EOF

awk -F@ \
'{ print "\t\tcase '${base}_e::'" $1 ": return \"" $1 "\";" }' "${infile}" >> "${outdir}/${base}.h"

cat >> "${outdir}/${base}.h" <<EOF
		default: return nullptr;
	}
}
$nsend

inline ::std::ostream & operator<<(
	::std::ostream & os_i,
	const ${nsprefix}${base}_e & ev_i
) {
	os_i << ${nsprefix}${base}_s(ev_i);
	return os_i;
}
EOF
