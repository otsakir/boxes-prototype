#include "utils.h"

#include <stdlib.h>

LogStream& LogStream::operator<<(const char* arg) {
    out << arg;
    out.flush();
    return *this;
}

LogStream& LogStream::operator<<(int arg) {
    out << arg;
    out.flush();
    return *this;
}

// from https://stackoverflow.com/questions/1202687/how-do-i-get-a-specific-range-of-numbers-from-rand
int randomInRange(int min, int max) {
    return rand() % (max + 1 - min) + min;
}

