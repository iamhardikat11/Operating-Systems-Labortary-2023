#include "goodmalloc.h"

void rec(int locI){
    functionStart();
    int ans = 0;
    getVal(locI, INT, &ans);
    if(ans ==-1){
        endScope();
        return;
    }else{
        int loc = createVar("Val",INT);
        printf(".........Printing value of argument now %d\n", ans);
        assignVal(loc,&ans,INT);
        addToVal(loc,-1);
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