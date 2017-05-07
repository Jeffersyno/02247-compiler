# Test file for using with bats: https://github.com/sstephenson/bats

# INSTALLATION
# Install bats in macOS by doing: $brew install bats
# for installation for other platforms check: https://github.com/sstephenson/bats/wiki/Install-Bats-Using-a-Package
#
# Pull git submodules with (use --init if you do it the first time):
#   `git submodule update [--init] --recursive`

# RUN
# From the command line: `bats tests.bats`

load 'testDependencies/bats-support/load'
load 'testDependencies/bats-assert/load'

RESULT="TEST"

OK="OK"
DEREF="DEREF"
NULL_DEREF="NULL_DEREF"
UNDEFINED_DEREF="UNDEFINED_DEREF"
ERROR="UNKNOWN_ERROR"
MISSED_DEFINITION="MISSED_DEFINITION"

function buildEventForInstruction() {
    echo "$RESULT[$2]:$1  $3"
}

function buildDereferenceRegexForInstruction() {
    buildEventForInstruction $1 $2 $3
}

function assert_events_count() {
    assert_equal $(echo $output | grep -o $RESULT | wc -l | tr -d " ") $1
}

function assert_nullderef_at_instruction() {
    assert_line $(buildDereferenceRegexForInstruction $NULL_DEREF $1 $2)
}

function assert_undefderef_at_instruction() {
    assert_line $(buildDereferenceRegexForInstruction $UNDEFINED_DEREF $1 $2)
}

# setup() {}

@test "basic/example0" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 5 "%4 = load i32, i32* %3, align 4"
}

@test "basic/example1" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 5 "%4 = load i32*, i32** %3, align 8"
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
  assert_nullderef_at_instruction 9 "%7 = load i32, i32* %6, align 4"
  assert_nullderef_at_instruction 12 "%9 = load i32, i32* %8, align 4"
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
  assert_nullderef_at_instruction 14 "%10 = load i32, i32* %9, align 4"
}

@test "basic/example6" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 12 "%8 = load i32, i32* %7, align 4"
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
  assert_nullderef_at_instruction 4 "store i32 5, i32* %2, align 4"
}

@test "basic/example9b" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 7 "store i32 %3, i32* %4, align 4"
}

@test "basic/example10" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 6 "%5 = load i32, i32* %4, align 4"
}

@test "basic/example11" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 5 "store i32 5, i32* %3, align 4"
}

@test "basic/example12" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 6
  assert_nullderef_at_instruction  19 "%13 = load i32**, i32*** %12, align 8"
  assert_undefderef_at_instruction 20 "%14 = load i32*, i32** %13, align 8"
  assert_undefderef_at_instruction 21 "%15 = load i32, i32* %14, align 4"
  assert_nullderef_at_instruction  25 "%18 = load i32**, i32*** %17, align 8"
  assert_undefderef_at_instruction 26 "%19 = load i32*, i32** %18, align 8"
  assert_undefderef_at_instruction 27 "%20 = load i32, i32* %19, align 4"
}

@test "basic/example13" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 0
}

@test "basic/example14" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction  10 "%8 = load i32, i32* %7, align 4"
}

@test "struct/example0" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 6 "%5 = load i32, i32* %4, align 4"
}

@test "struct/example1" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 5 "store i32 77, i32* %3, align 4"
}

@test "struct/example2" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 7 "call void @llvm.memcpy.p0i8.p0i8.i64(i8* %4, i8* %5, i64 4, i32 4, i1 false)"
}

@test "struct/example3" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 7 "%6 = load i32, i32* %5, align 4"
}

@test "struct/example4" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 2
  assert_nullderef_at_instruction 12 "%10 = load i32, i32* %9, align 4"
  assert_nullderef_at_instruction 16 "%13 = load i32, i32* %12, align 4"
}

@test "struct/example5" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 22 "%17 = load i32, i32* %16, align 4"
}

@test "flow/example1" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_success
}

@test "flow/example2" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 5 "%4 = load i32, i32* %3, align 4"
}

@test "flow/example3" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 10 "%7 = load i32, i32* %6, align 4"
}

@test "flow/example4" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 10 "%7 = load i32, i32* %6, align 4"
}

@test "flow/example5" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 2
  assert_nullderef_at_instruction 13 "%9 = load i32, i32* %8, align 4"
  assert_nullderef_at_instruction 17 "%11 = load i32, i32* %10, align 4"
}

@test "flow/example6" {
  run ./run $BATS_TEST_DESCRIPTION
  assert_failure

  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 1
  assert_nullderef_at_instruction 8 "%5 = load i32, i32* %4, align 4"
}

@test "others/array_unknown_indices" {
  run ./opt $BATS_TEST_DESCRIPTION
  assert_events_count 0
}