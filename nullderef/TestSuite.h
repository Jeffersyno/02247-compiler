//
// Created by Andrés Cecilia on 19/3/17.
//

#ifndef TESTSUITE_H
#define TESTSUITE_H

#include "llvm/Support/raw_ostream.h"

#include "PointerTrackerVisitor.h"

using namespace llvm;

class TestSuite {
    static std::string readableOutput(VisitResult result) {
        switch(result) {
            case OK: return "Ok";
            case NULL_DEREF: return "Null dereference";
            case MAYBE_NULL_DEREF: return "Possible null dereference";
            case UNKNOWN_ERROR: return "Unknown error";
        };
    }

    static std::string outputID(VisitResult result) {
        switch(result) {
            case OK: return "OK";
            case NULL_DEREF: return "NULL_DEREF";
            case MAYBE_NULL_DEREF: return "MAYBE_NULL_DEREF";
            case UNKNOWN_ERROR: return "UNKNOWN_ERROR";
        };
    }

    static std::string resultOutput(VisitResult result) {
        return "RESULT: " + TestSuite::outputID(result);
    }

public:
    static void printResult(VisitResult result, Instruction& instruction) {
        errs() << resultOutput(result) << " at ";
        instruction.dump();
    }

    static void printResult(VisitResult result) {
        errs() << resultOutput(result) << "\n";
    }
};

#endif //TESTSUITE_H
