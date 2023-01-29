/*
    Demonstrates type-checking
*/

#include "memlab.h"

using namespace std;

#define EXCEPTION(msg, ...) printf("\x1b[31m[EXCEPTION] " msg " \x1b[0m\n", ##__VA_ARGS__);

int main() {
    createMem(400, true);
    initScope();

    try {
        MyType v1 = createVar(CHAR);
        medium_int m1(12);
        assignVar(v1, m1);
    } catch (const runtime_error& e) {
        EXCEPTION("%s", e.what());
    }

    try {
        MyType v2 = createVar(BOOLEAN);
        int i1 = 12;
        assignVar(v2, i1);
    } catch (const runtime_error& e) {
        EXCEPTION("%s", e.what());
    }

    try {
        MyType v3 = createArr(BOOLEAN, 10);
        int i3 = 5;
        assignArr(v3, 10, i3);
    } catch (const runtime_error& e) {
        EXCEPTION("%s", e.what());
    }

    try {
        MyType v4 = createArr(INT, 10);
        int i4 = 5;
        assignArr(v4, 10, i4);
    } catch (const runtime_error& e) {
        EXCEPTION("%s", e.what());
    }

    endScope();
    cleanExit();
}