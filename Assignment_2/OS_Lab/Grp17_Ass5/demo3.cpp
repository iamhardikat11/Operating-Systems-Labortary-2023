#include"memlab.h"

void rec(int locI){
    MemLab::functionStart();

    if(MemLab::getValueVarInt(locI)==-1){
        MemLab::endScope();
        return;
    }else{
        int loc = MemLab::createVar("Val",INT);
        cout<<".........Printing value of argument now "<<MemLab::getValueVarInt(locI)<<"\n";
        MemLab::assignVar(loc,MemLab::getValueVarInt(locI));
        MemLab::addToVar(loc,-1);
        rec(loc);
    }

    MemLab::endScope();
}

int main(){
    MemLab::createMem();
    int loc = MemLab::createVar("Val",INT);
    MemLab::assignVar(loc,5);
    rec(loc);
    MemLab::endScope();
}