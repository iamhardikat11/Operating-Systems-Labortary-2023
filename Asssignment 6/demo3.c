#include "goodmalloc.h"

void rec(int locI){
    functionStart();

    if(getValueVarInt(locI)==-1){
        endScope();
        return;
    }else{
        int loc = createVar("Val",INT);
        printf(".........Printing value of argument now %d\n", getValueVarInt(locI));
        int ans = getValueVarInt(locI);
        assignVal(loc,&ans,INT);
        addToVal(loc,-1);
        rec(loc);
    }
    endScope();
}

int main(){
    createMem();
    int loc = createVar("Val",INT);
    assignVal(loc,&loc,INT);
    rec(loc);
    endScope();
}