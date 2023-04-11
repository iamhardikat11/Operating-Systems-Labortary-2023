#include "goodmalloc.h"

int main()
{
    createMem();
    int locK = createVar("K", INT);
    int x = 0;
    assignVal(locK, &x, INT);

    // int ret = createVar("Ans", INT);

    // fibonacci(locK,ret);

    // printf("Ans: %d\n", getValueVarInt(ret));

    endScope();
    return 0;
}