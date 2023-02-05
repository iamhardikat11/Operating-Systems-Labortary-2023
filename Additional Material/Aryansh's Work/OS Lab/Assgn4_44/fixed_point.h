#include<stdint.h>

#define P 17
#define Q 14
#define F 16384
#define fxpt_t int
#define int64 long long int

fxpt_t int_to_fxpt(int n);
int fxpt_to_int_trunc(fxpt_t x);
int fxpt_to_int_round(fxpt_t x);
fxpt_t add_fxpt(fxpt_t x,fxpt_t y);
fxpt_t sub_fxpt(fxpt_t x,fxpt_t y);
fxpt_t add_int(fxpt_t x,int n);
fxpt_t sub_int(fxpt_t x,int n);
fxpt_t mul_fxpt(fxpt_t x,fxpt_t y);
fxpt_t mul_int(fxpt_t x,int n);
fxpt_t div_fxpt(fxpt_t x,fxpt_t y);
fxpt_t div_int(fxpt_t x,int n);

fxpt_t int_to_fxpt(int n){
    return (n*F);
}

int fxpt_to_int_trunc(fxpt_t x){
    return (x/F);
}

int fxpt_to_int_round(fxpt_t x){
    if(x>=0){
        return (x+F/2)/F;
    }
    return (x-F/2)/F;
}

fxpt_t add_fxpt(fxpt_t x,fxpt_t y){
    return (x+y);
}

fxpt_t sub_fxpt(fxpt_t x,fxpt_t y){
    return (x-y);
}

fxpt_t add_int(fxpt_t x,int n){
    return (x+n*F);
}

fxpt_t sub_int(fxpt_t x,int n){
    return (x-n*F);
}

fxpt_t mul_fxpt(fxpt_t x,fxpt_t y){
    return ((int64)x)*y/F;
}

fxpt_t mul_int(fxpt_t x,int n){
    return x*n;
}

fxpt_t div_fxpt(fxpt_t x,fxpt_t y){
    return ((int64)x)* F/y;
}

fxpt_t div_int(fxpt_t x,int n){
    return x/n;
}