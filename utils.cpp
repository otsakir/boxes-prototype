#include "utils.h"


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

