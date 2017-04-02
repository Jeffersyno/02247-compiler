#ifndef ERROR_CODE_H
#define ERROR_CODE_H 1

#include <string>

enum ErrorCode {
    /** All is well. */
    OK = 0,

    /** An invalid value has been dereferenced. */
    DEREF = 16,

    /** A null value has been dereferenced. */
    NULL_DEREF = DEREF | 1,

    /** An undefined value has been dereferenced. */
    UNDEFINED_DEREF = DEREF | 2,

    /** Something went wrong, but we aren't sure exactly what went wrong. */
    ERROR = 32,

    /** We expected a key/value pair in our map, but we couldn't it. */
    MISSED_DEFINITION = ERROR | 1
};

std::string errorCodeName(ErrorCode code) {
    switch (code) {
    case OK:                return "OK";
    case DEREF:             return "DEREF";
    case NULL_DEREF:        return "NULL_DEREF";
    case UNDEFINED_DEREF:   return "UNDEFINED_DEREF";
    case ERROR:             return "UNKNOWN_ERROR";
    case MISSED_DEFINITION: return "MISSED_DEFINITION";
    default:                return "???";
    }
}

#endif // ERROR_CODE_H
