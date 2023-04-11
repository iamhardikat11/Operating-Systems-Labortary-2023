#include "goodmalloc.h"

void function1(int locX,int locY){
    functionStart();

    int loc=createArr("Arr",INT,500000);

    for(int i=0;i<50000;i++){
        assignArrInt(loc,i,rand()%100000);
    }

    endScope();
}

void function2(int locX,int locY){
    functionStart();

    int loc=createArr("Arr",CHAR,50000);

    for(int i=0;i<50000;i++){
        assignArrChar(loc,i,(char)('a'+rand()%25));
    }

    endScope();
}

void function3(int locX,int locY){
    functionStart();

    int loc=createArr("Arr",BOOLEAN,50000);

    for(int i=0;i<50000;i++){
        assignArrBool(loc,i,(bool)(rand()%2));
    }

    endScope();
}

void function4(int locX,int locY){
    functionStart();

    int loc=createArr("Arr",MEDIUM_INT,50000);

    for(int i=0;i<50000;i++){
        assignArrMedium(loc,i,CreateMediumInt(rand()%100000));
    }

    endScope();
}

int main(){
    srand(time(0));
    createMem();

    int locX=createVar("X1",INT);
    int locY=createVar("Y1",INT);
    function1(locX,locY);

    locX=createVar("X2",INT);
    locY=createVar("Y2",INT);
    function1(locX,locY);

    locX=createVar("X3",CHAR);
    locY=createVar("Y3",CHAR);
    function2(locX,locY);

    locX=createVar("X4",CHAR);
    locY=createVar("Y4",CHAR);
    function2(locX,locY);

    locX=createVar("X5",CHAR);
    locY=createVar("Y5",CHAR);
    function2(locX,locY);

    locX=createVar("X6",BOOLEAN);
    locY=createVar("Y6",BOOLEAN);
    function3(locX,locY);

    locX=createVar("X7",BOOLEAN);
    locY=createVar("Y7",BOOLEAN);
    function3(locX,locY);

    locX=createVar("X8",BOOLEAN);
    locY=createVar("Y8",BOOLEAN);
    function3(locX,locY);

    locX=createVar("X9",MEDIUM_INT);
    locY=createVar("Y9",MEDIUM_INT);
    function4(locX,locY);

    locX=createVar("X10",MEDIUM_INT);
    locY=createVar("Y10",MEDIUM_INT);
    function4(locX,locY);

    endScope();

    return 0;
}
