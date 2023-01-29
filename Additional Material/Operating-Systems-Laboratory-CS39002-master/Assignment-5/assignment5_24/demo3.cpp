/*
    Demonstrates the compaction scheme of using the ratio totalFree/currMaxFree
*/

#include "memlab.h"

using namespace std;

int main() {
    createMem(400, true);
    initScope();
    MyType v1 = createVar(INT);
    MyType arr1 = createArr(INT, 10);
    MyType arr2 = createArr(CHAR, 40);
    MyType v2 = createVar(MEDIUM_INT);
    MyType v3 = createVar(BOOLEAN);
    MyType arr3 = createArr(MEDIUM_INT, 8);
    MyType arr4 = createArr(BOOLEAN, 320);
    MyType v4 = createVar(INT);
    MyType v5 = createVar(CHAR);
    MyType arr5 = createArr(INT, 12);
    MyType arr6 = createArr(CHAR, 12);
    MyType v6 = createVar(INT);
    MyType arr7 = createArr(INT, 12);
    MyType v7 = createVar(CHAR);
    MyType v8 = createVar(MEDIUM_INT);

    freeElem(v1);
    freeElem(arr2);
    freeElem(v3);
    freeElem(arr4);
    freeElem(v5);
    freeElem(arr6);
    freeElem(arr7);
    usleep(20);
    endScope();
    cleanExit();
}