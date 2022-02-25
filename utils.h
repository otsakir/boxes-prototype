#ifndef _UTILS_H_
#define _UTILS_H_

#include <iostream>


class LogStream {
private:
	std::ostream& out;
public:

	LogStream(std::ostream& out) : out(out) {}

	LogStream& operator<<(const char* arg);
    
	LogStream& operator<<(int arg);
    
};


#endif
