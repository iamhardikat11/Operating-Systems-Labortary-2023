#include"memlab.h"

void rec(int locI){
    functionStart();

    if(getValueVarInt(locI)==-1){
        endScope();
        return;
    }else{
        int loc = createVar("Val",INT);
        cout<<".........Printing value of argument now "<<getValueVarInt(locI)<<"\n";
        assignVar(loc,getValueVarInt(locI));
        addToVar(loc,-1);
        rec(loc);
    }

    endScope();
}

int main(){
    createMem();
    int loc = createVar("Val",INT);
    assignVar(loc,5);
    rec(loc);
    endScope();
}