#ifndef __TEST_EXCEPTION_H_GUARD__
#define __TEST_EXCEPTION_H_GUARD__

#include <exception>
class TestException : public std::exception {
private:
    const char* message;
public:
    explicit TestException(const char* m) : message(m) {}
    const char* what() const {
        return message;
    }
};

#endif

