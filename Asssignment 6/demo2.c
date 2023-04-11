#include"memlab.h"

void fibonacci(int locK,int ret){
    functionStart();

    int arrLoc=createArr("Fib",INT,getValueVarInt(locK));

    assignArrInt(arrLoc,0,1);
    assignArrInt(arrLoc,1,1);

    for(int i=2;i<getValueVarInt(locK);i++){
        assignArrInt(arrLoc,i,getValueArrInt(arrLoc,i-1));
        addToArr(arrLoc,i,getValueArrInt(arrLoc,i-2));
    }

    int prodLoc=createVar("Prod",INT);

    assignVarInt(prodLoc,1);

    for(int i=0;i<getValueVarInt(locK);i++){
        multToVar(prodLoc,getValueArrInt(arrLoc,i));
    }

    assignVarInt(ret,getValueVarInt(prodLoc));

    endScope();
}

int main(){
    createMem();

    int locK=createVar("K",INT);

    assignVarInt(locK,10);

    int ret=createVar("Ans",INT);

    fibonacci(locK,ret);

    printf("Ans: %d\n", getValueVarInt(ret));

    endScope();
    return 0;
}