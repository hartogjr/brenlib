# @author    Bren de Hartog <bren@dehartog.name>
# @copyright Copyright (c) 2026, Bren de Hartog. All rights reserved.
# @license   This project is licensed under the 3-clause BSD license:
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
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

# Settings
set shell := ["bash", "-uc"]
set positional-arguments := true

# Default action is to list Just recipes
default:
	@just --list

# Debug build alias
bd: (build "debug")

# Release build alias
br: (build "release")

# Build debug or release version
build target="debug": (_checkcmd "cmake") (conan target)
	#!/usr/bin/env bash
	set -euo pipefail
	[ "{{target}}" = "debug" ] || [ "{{target}}" = "release" ]
	# The CMAKE_BUILD_TYPE setting has already been set by Conan
	# The EXPORT_COMPILE_COMMANDS is for clangd LSP to work much better
	if ! test -r {{justfile_directory()}}/bld/{{target}}/Makefile
	then
		cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 --preset conan-{{ target }} \
		-S {{justfile_directory()}} -B {{justfile_directory()}}/bld/{{target}}
	fi
	ln -sf {{justfile_directory()}}/bld/{{target}}/compile_commands.json \
		{{justfile_directory()}}/compile_commands.json
	cmake --build {{justfile_directory()}}/bld/{{target}} -j$(nproc)

# Check for command existence and report if not present
_checkcmd exe:
	#!/usr/bin/env bash
	set -euo pipefail
	if ! command -v {{ exe }} &>/dev/null
	then
		echo "The command '{{ exe }}' is not available, please install it first"
		exit 1
	fi

# Configure and run Conan to install requested dependencies
conan target="debug": (_checkcmd "conan")
	#!/usr/bin/env bash
	set -euo pipefail
	deflt="${HOME}/.conan2/profiles/default"
	# Apparently, Conan also needs a default profile, even if we don't use it
	if ! conan profile path default &>/dev/null
	then
		mkdir -p "$(dirname "${deflt}")"
		conan profile detect -verror
	fi
	if ! conan profile path {{ target }} &>/dev/null
	then
		tgt="${HOME}/.conan2/profiles/{{ target }}"
		cp "${dflt}" "${tgt}"
		sed -i -r \
			-e 's/^build_type=[A-Z][a-z]+/build_type={{capitalize(target)}}/g' \
			-e 's/compiler\.cppstd=[a-z0-9]+/compiler.cppstd=gnu20/g' \
			"${tgt}"
	fi
	mkdir -p {{justfile_directory()}}/bld/{{target}}
	conan install "{{ justfile_directory() }}" --profile:all={{ target }} \
		"--output-folder={{ justfile_directory() }}/bld/{{ target }}" \
		--build=missing
