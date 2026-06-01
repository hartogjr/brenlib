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
build target="debug":
	#!/usr/bin/env bash
	set -euo pipefail
	[ "{{target}}" = "debug" ] || [ "{{target}}" = "release" ]
	mkdir -p {{justfile_directory()}}/bld/{{target}}
	export CC=$(command -v clang)
	export CXX=$(command -v clang++)
	# The EXPORT_COMPILE_COMMANDS is for clangd LSP to work much better
	# The POLICY_VERSION_MINIMUM is to fix SQLiteCpp compilation
	test -r {{justfile_directory()}}/bld/{{target}}/Makefile || \
		cmake -DCMAKE_BUILD_TYPE={{capitalize(target)}} \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S {{justfile_directory()}} \
		-B {{justfile_directory()}}/bld/{{target}}
	ln -sf {{justfile_directory()}}/bld/{{target}}/compile_commands.json \
		{{justfile_directory()}}/compile_commands.json
	make -C {{justfile_directory()}}/bld/{{target}} -j$(nproc)
