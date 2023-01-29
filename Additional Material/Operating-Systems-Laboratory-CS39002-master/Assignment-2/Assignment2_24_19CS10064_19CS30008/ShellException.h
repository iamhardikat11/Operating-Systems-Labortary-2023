#ifndef __SHELLEXCEPTION_H
#define __SHELLEXCEPTION_H

#include <exception>
#include <string>

using namespace std;

class ShellException : public exception {
   protected:
    const string message;

   public:
    ShellException(const string&);
    virtual const char* what() const throw();
};

#endif