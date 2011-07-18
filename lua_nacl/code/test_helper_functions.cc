// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is an example of a simple unit test to verify that the logic helper
// functions works as expected.  Note that this looks like a 'normal' C++
// program, with a main function.  It is compiled and linked using the NaCl
// toolchain, so in order to run it, you must use 'nacl-sel_ldr' or
// 'nacl64-sel_ldr' from the toolchain's bin directory.
//
// For example (assuming the toolchain bin directory is in your path):
// > nacl-sel_ldr test_helper_functions_x86_32_dbg.nexe
//
// You can also use the 'test', 'test32', or 'test64' make target to run this
// test.  For example, this will run the test in both 32-bit and 64-bit mode:
// > make test

#include "code/helper_functions.h"

#include <cassert>
#include <cstdio>
#include <string>

// A very simple macro to print 'passed' if boolean_expression is true and
// 'FAILED' otherwise.
// This is meant to approximate the functionality you would get from a real test
// framework.  You should feel free to build and use the test framework of your
// choice.
#define EXPECT_EQUAL(left, right)\
printf("Check: \"" #left "\" == \"" #right "\" %s\n", \
       ((left) == (right)) ? "passed" : "FAILED")

using hello_world::FortyTwo;
using hello_world::ReverseText;

int main() {
  EXPECT_EQUAL(FortyTwo(), 42);

  std::string empty_string;
  EXPECT_EQUAL(ReverseText(empty_string), empty_string);

  std::string palindrome("able was i ere i saw elba");
  EXPECT_EQUAL(ReverseText(palindrome), palindrome);

  std::string alphabet("abcdefghijklmnopqrstuvwxyz");
  std::string alphabet_backwards("zyxwvutsrqponmlkjihgfedcba");
  EXPECT_EQUAL(ReverseText(alphabet), alphabet_backwards);
}

