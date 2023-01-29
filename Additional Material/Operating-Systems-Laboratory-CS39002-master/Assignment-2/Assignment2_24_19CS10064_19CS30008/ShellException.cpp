#include "ShellException.h"

ShellException::ShellException(const string& msg) : message(msg) {}

const char* ShellException::what() const throw() {
    return message.c_str();
}