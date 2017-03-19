#!/bin/bash
# Test file for using with bats: https://github.com/sstephenson/bats
# Install bats in macOS doing: 
#	$brew install bats
# Install needed addons (bats-assert: https://github.com/ztombol/bats-assert):
#	$brew tap kaos/shell
#	$brew install bats-assert
# run with: $bats tests.bats

TEST_BREW_PREFIX="$(brew --prefix)"
load 'test/bats-support/load'
load 'test/bats-assert/load'

RESULT_OUTPUT="RESULT:"

OK="OK"
NULL_DEREF="NULL_DEREF"
MAYBE_NULL_DEREF="MAYBE_NULL_DEREF"
UNKNOWN_ERROR="UNKNOWN_ERROR"

function createResult() {
	echo $RESULT_OUTPUT $1 "at  " $2
}

function createDerefResult() {
	echo $RESULT_OUTPUT $NULL_DEREF "at  " $1
}

@test "Example 1" {
  run ./opt example1
  instruction="%7 = load i32, i32* %6, align 4"
  assert_line $(createDerefResult $instruction)

  instruction="%9 = load i32, i32* %8, align 4"
  assert_line $(createDerefResult $instruction)
}

@test "Example 2" {
  run ./opt example2
  instruction="%3 = load i32, i32* %2, align 4"
  assert_line $(createDerefResult $instruction)
}
