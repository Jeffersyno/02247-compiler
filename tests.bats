#!/bin/bash
# Test file for using with bats: https://github.com/sstephenson/bats

# INSTALLATION
# Install bats in macOS by doing: $brew install bats
# for installation for other platforms check: https://github.com/sstephenson/bats/wiki/Install-Bats-Using-a-Package
#
# Pull git submodules with (use --init if you do it the first time):
#   `git submodule update [--init] --recursive`

# RUN
# From the command line: `bats tests.bats`

load 'test/bats-support/load'
load 'test/bats-assert/load'

RESULT_OUTPUT="RESULT:"

OK="OK"
NULL_DEREF="NULL_DEREF"
MAYBE_NULL_DEREF="MAYBE_NULL_DEREF"
UNKNOWN_ERROR="UNKNOWN_ERROR"

function buildEventForInstruction() {
  echo $RESULT_OUTPUT$1";INDEX:"$2";INSTRUCTION:  "$3
}

function buildDereferenceRegexForInstruction() {
  buildEventForInstruction $NULL_DEREF $1 $2
}

function assert_events_count() {
	assert_equal $(echo $output | grep -o $RESULT_OUTPUT | wc -l | tr -d " ") $1
}

function assert_dereference_at_instruction() {
  	assert_line $(buildDereferenceRegexForInstruction $1 $2)
}

# setup() {}

@test "basic/example0" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 5 "%4 = load i32, i32* %3, align 4"
}

@test "basic/example1" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 5 "%4 = load i32*, i32** %3, align 8"
}

@test "basic/example2" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_success

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 0
}

@test "basic/example3" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 2
  assert_dereference_at_instruction 9 "%7 = load i32, i32* %6, align 4"
  assert_dereference_at_instruction 12 "%9 = load i32, i32* %8, align 4"
}

@test "basic/example4" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_success

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 0
}

@test "basic/example5" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 14 "%10 = load i32, i32* %9, align 4"
}

@test "basic/example6" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 12 "%8 = load i32, i32* %7, align 4"
}

@test "basic/example7" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_success

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 0
}

@test "basic/example8" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_success

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 0
}

@test "basic/example9" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 4 "store i32 5, i32* %2, align 4"
}

@test "basic/example10" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 4 "%5 = load i32, i32* %4, align 4"
}

@test "basic/example11" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 5 "store i32 5, i32* %3, align 4"
}

@test "struct/example0" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 6 "%5 = load i32, i32* %4, align 4"
}

@test "struct/example1" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 5 "store i32 77, i32* %3, align 4"
}

@test "struct/example2" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_dereference_at_instruction 7 "call void @llvm.memcpy.p0i8.p0i8.i64(i8* %4, i8* %5, i64 4, i32 4, i1 false)"
}

