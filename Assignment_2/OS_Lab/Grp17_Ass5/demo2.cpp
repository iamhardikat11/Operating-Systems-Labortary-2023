#include"memlab.h"

void fibonacci(int locK,int ret){
    MemLab::functionStart();

    int arrLoc=MemLab::createArr("Fib",INT,MemLab::getValueVarInt(locK));

    MemLab::assignArr(arrLoc,0,1);
    MemLab::assignArr(arrLoc,1,1);

    for(int i=2;i<MemLab::getValueVarInt(locK);i++){
        MemLab::assignArr(arrLoc,i,MemLab::getValueArrInt(arrLoc,i-1));
        MemLab::addToArr(arrLoc,i,MemLab::getValueArrInt(arrLoc,i-2));
    }

    int prodLoc=MemLab::createVar("Prod",INT);

    MemLab::assignVar(prodLoc,1);

    for(int i=0;i<MemLab::getValueVarInt(locK);i++){
        MemLab::multToVar(prodLoc,MemLab::getValueArrInt(arrLoc,i));
    }

    MemLab::assignVar(ret,MemLab::getValueVarInt(prodLoc));

    MemLab::endScope();
}

int main(){
    MemLab::createMem();

    int locK=MemLab::createVar("K",INT);

    MemLab::assignVar(locK,10);

    int ret=MemLab::createVar("Ans",INT);

    fibonacci(locK,ret);
        
    cout<<"Ans: "<<MemLab::getValueVarInt(ret)<<"\n";

    MemLab::endScope();
    return 0;
}