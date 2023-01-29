/*
    Implements a fibonacci function using memory management. The main takes a parameter k,
    and passes it to a function fibonacciProduct which calls the fibonacci to populate an array
    of the first-k fibonacci numbers, computes its product and returns the product.
*/

#include "memlab.h"

using namespace std;

void fibonacci(MyType fib, MyType k) {
    initScope();
    int k_val;
    readVar(k, &k_val);
    for (int i = 0; i < k_val; i++) {
        if (i == 0) {
            assignArr(fib, 0, 1);
        } else if (i == 1) {
            assignArr(fib, 1, 1);
        } else {
            int fib_i_1, fib_i_2;
            readArr(fib, i - 1, &fib_i_1);
            readArr(fib, i - 2, &fib_i_2);
            assignArr(fib, i, fib_i_1 + fib_i_2);
        }
    }
    endScope();
    gcActivate();
}

int fibonacciProduct(MyType k) {
    initScope();
    int k_val;
    readVar(k, &k_val);
    MyType fib = createArr(INT, k_val);
    fibonacci(fib, k);
    long long prod = 1;
    for (int i = 0; i < k_val; i++) {
        int fib_val;
        readArr(fib, i, &fib_val);
        // printf("%d\n", fib_val);
        prod *= fib_val;
    }
    endScope();
    gcActivate();
    return prod;
}

int main(int argc, char* argv[]) {
    createMem(500 * 1024 * 1024, true);
    initScope();
    MyType k = createVar(INT);
    if (argc < 2) {
        assignVar(k, 5);
    } else {
        assignVar(k, atoi(argv[1]));
    }
    int k_val;
    readVar(k, &k_val);
    long long prod = fibonacciProduct(k);
    printf("The product of the first %d fibonacci numbers is %lld\n", k_val, prod);
    endScope();
    gcActivate();
    cleanExit();
}