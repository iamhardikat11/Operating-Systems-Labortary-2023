#include"memlab.h"

void rec(int locI){
    functionStart();

    if(getValueVarInt(locI)==-1){
        endScope();
        return;
    }else{
        int loc = createVar("Val",INT);
        printf(".........Printing value of argument now %d\n", getValueVarInt(locI));
        assignVarInt(loc,getValueVarInt(locI));
        addToVar(loc,-1);
        rec(loc);
    }

    endScope();
}

int main(){
    createMem();
    int loc = createVar("Val",INT);
    assignVarInt(loc,5);
    rec(loc);
    endScope();
}