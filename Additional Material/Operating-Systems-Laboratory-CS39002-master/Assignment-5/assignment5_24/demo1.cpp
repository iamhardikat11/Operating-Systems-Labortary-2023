/*
    The main function takes 250 MB memory. Then it calls 10 functions with
    two parameters x and y (x and y same data type). Each function creates
    and populates an array of 50000 elements of the same data type with random data,
    destroys the array and returns
*/

#include "memlab.h"

using namespace std;

const int ARR_SIZE = 50000;

void func1(MyType x, MyType y) {  // INT
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        int sign = i % 2 == 0 ? 1 : -1;
        assignArr(arr, i, sign * rand());
    }
    endScope();
    gcActivate();
}

void func2(MyType x, MyType y) {  // MEDIUM INT
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        int sign = i % 2 == 0 ? 1 : -1;
        assignArr(arr, i, medium_int(sign * (rand() % (1 << 23))));
    }
    endScope();
    gcActivate();
}

void func3(MyType x, MyType y) {  // CHAR
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        assignArr(arr, i, (char)('a' + (rand() % 26)));
    }
    endScope();
    gcActivate();
}

void func4(MyType x, MyType y) {  // BOOL
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        assignArr(arr, i, (bool)(rand() % 2));
    }
    endScope();
    gcActivate();
}

void func5(MyType x, MyType y) {  // INT
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        int sign = i % 2 == 0 ? 1 : -1;
        assignArr(arr, i, sign * rand());
    }
    endScope();
    gcActivate();
}

void func6(MyType x, MyType y) {  // MEDIUM INT
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        int sign = i % 2 == 0 ? 1 : -1;
        assignArr(arr, i, medium_int(sign * (rand() % (1 << 23))));
    }
    endScope();
    gcActivate();
}

void func7(MyType x, MyType y) {  // CHAR
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        assignArr(arr, i, (char)('a' + (rand() % 26)));
    }
    endScope();
    gcActivate();
}

void func8(MyType x, MyType y) {  // BOOL
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        assignArr(arr, i, (bool)(rand() % 2));
    }
    endScope();
    gcActivate();
}

void func9(MyType x, MyType y) {  // INT
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        int sign = i % 2 == 0 ? 1 : -1;
        assignArr(arr, i, sign * rand());
    }
    endScope();
    gcActivate();
}

void func10(MyType x, MyType y) {  // CHAR
    initScope();
    MyType arr = createArr(x.data_type, ARR_SIZE);
    for (int i = 0; i < ARR_SIZE; i++) {
        assignArr(arr, i, (char)('a' + (rand() % 26)));
    }
    endScope();
    gcActivate();
}

int main() {
    srand(time(NULL));
    createMem(250 * 1024 * 1024, true);
    initScope();
    MyType x1 = createVar(INT);
    assignVar(x1, 16);
    MyType y1 = createVar(INT);
    assignVar(y1, -39);
    MyType x2 = createVar(MEDIUM_INT);
    assignVar(x2, medium_int(22));
    MyType y2 = createVar(MEDIUM_INT);
    assignVar(y2, medium_int(-95));
    MyType x3 = createVar(CHAR);
    assignVar(x3, 'a');
    MyType y3 = createVar(CHAR);
    assignVar(y3, 'z');
    MyType x4 = createVar(BOOLEAN);
    assignVar(x4, true);
    MyType y4 = createVar(BOOLEAN);
    assignVar(y4, false);

    func1(x1, y1);
    func2(x2, y2);
    func3(x3, y3);
    func4(x4, y4);
    func5(x1, y1);
    func6(x2, y2);
    func7(x3, y3);
    func8(x4, y4);
    func9(x1, y1);
    func10(x3, y3);

    endScope();
    gcActivate();
    cleanExit();
}