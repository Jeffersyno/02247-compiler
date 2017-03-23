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
            case MISSED_DEFINITION: return "Error, previous needed instruction not found in the map";
            case UNKNOWN_ERROR: return "Unknown error";
        };
    }

    static std::string outputID(VisitResult result) {
        switch(result) {
            case OK: return "OK";
            case NULL_DEREF: return "NULL_DEREF";
            case MAYBE_NULL_DEREF: return "MAYBE_NULL_DEREF";
            case MISSED_DEFINITION: return "MISSED_DEFINITION";
            case UNKNOWN_ERROR:
        default: return "UNKNOWN_ERROR"; break;
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
