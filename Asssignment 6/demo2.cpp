#include"memlab.h"
#include <iostream>
using namespace std;

void fibonacci(int locK,int ret){
    functionStart();

    int arrLoc=createArr("Fib",INT,getValueVarInt(locK));

    assignArr(arrLoc,0,1);
    assignArr(arrLoc,1,1);

    for(int i=2;i<getValueVarInt(locK);i++){
        assignArr(arrLoc,i,getValueArrInt(arrLoc,i-1));
        addToArr(arrLoc,i,getValueArrInt(arrLoc,i-2));
    }

    int prodLoc=createVar("Prod",INT);

    assignVar(prodLoc,1);

    for(int i=0;i<getValueVarInt(locK);i++){
        multToVar(prodLoc,getValueArrInt(arrLoc,i));
    }

    assignVar(ret,getValueVarInt(prodLoc));

    endScope();
}

int main(){
    createMem();

    int locK=createVar("K",INT);

    assignVar(locK,10);

    int ret=createVar("Ans",INT);

    fibonacci(locK,ret);
        
    cout<<"Ans: "<<getValueVarInt(ret)<<"\n";

    endScope();
    return 0;
}