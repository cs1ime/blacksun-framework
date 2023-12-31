// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE.txt for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once


// determine the compiler/toolchain used
#if defined(__clang__)
#	define PDB_COMPILER_MSVC				0
#	define PDB_COMPILER_CLANG				1
#	define PDB_COMPILER_GCC					0
#elif defined(_MSC_VER)
#	define PDB_COMPILER_MSVC				1
#	define PDB_COMPILER_CLANG				0
#	define PDB_COMPILER_GCC					0
#elif defined(__GNUC__)
#	define PDB_COMPILER_MSVC				0
#	define PDB_COMPILER_CLANG				0
#	define PDB_COMPILER_GCC					1
#else
#	error("Unknown compiler.");
#endif


#ifdef _MSC_VER

#else
#define __noop 
#endif


// check whether C++17 is available
#if __cplusplus >= 201703L
#	define PDB_CPP_17						1
#else
#	define PDB_CPP_17						0
#endif
