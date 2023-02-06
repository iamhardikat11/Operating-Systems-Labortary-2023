#include "memlab.h"

void function1(int locX,int locY){
    MemLab::functionStart();

    int loc=MemLab::createArr("Arr",INT,500000);

    for(int i=0;i<50000;i++){
        MemLab::assignArr(loc,i,rand()%100000);
    }

    MemLab::endScope();
}

void function2(int locX,int locY){
    MemLab::functionStart();

    int loc=MemLab::createArr("Arr",CHAR,50000);

    for(int i=0;i<50000;i++){
        MemLab::assignArr(loc,i,(char)('a'+rand()%25));
    }

    MemLab::endScope();
}

void function3(int locX,int locY){
    MemLab::functionStart();

    int loc=MemLab::createArr("Arr",BOOLEAN,50000);

    for(int i=0;i<50000;i++){
        MemLab::assignArr(loc,i,(bool)(rand()%2));
    }

    MemLab::endScope();
}

void function4(int locX,int locY){
    MemLab::functionStart();

    int loc=MemLab::createArr("Arr",MEDIUM_INT,50000);

    for(int i=0;i<50000;i++){
        MemLab::assignArr(loc,i,mediumInt(rand()%100000));
    }

    MemLab::endScope();
}

int main(){
    srand(time(0));
    MemLab::createMem();

    int locX=MemLab::createVar("X1",INT);
    int locY=MemLab::createVar("Y1",INT);
    function1(locX,locY);

    locX=MemLab::createVar("X2",INT);
    locY=MemLab::createVar("Y2",INT);
    function1(locX,locY);

    locX=MemLab::createVar("X3",CHAR);
    locY=MemLab::createVar("Y3",CHAR);
    function2(locX,locY);

    locX=MemLab::createVar("X4",CHAR);
    locY=MemLab::createVar("Y4",CHAR);
    function2(locX,locY);

    locX=MemLab::createVar("X5",CHAR);
    locY=MemLab::createVar("Y5",CHAR);
    function2(locX,locY);

    locX=MemLab::createVar("X6",BOOLEAN);
    locY=MemLab::createVar("Y6",BOOLEAN);
    function3(locX,locY);

    locX=MemLab::createVar("X7",BOOLEAN);
    locY=MemLab::createVar("Y7",BOOLEAN);
    function3(locX,locY);

    locX=MemLab::createVar("X8",BOOLEAN);
    locY=MemLab::createVar("Y8",BOOLEAN);
    function3(locX,locY);

    locX=MemLab::createVar("X9",MEDIUM_INT);
    locY=MemLab::createVar("Y9",MEDIUM_INT);
    function4(locX,locY);

    locX=MemLab::createVar("X10",MEDIUM_INT);
    locY=MemLab::createVar("Y10",MEDIUM_INT);
    function4(locX,locY);

    MemLab::endScope();

    return 0;
}
