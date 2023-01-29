#ifndef __MEMLAB_H
#define __MEMLAB_H

#include <unistd.h>

#include <iostream>
#include <string>

using namespace std;

#define WARNING(msg, ...) printf("\x1b[35m[WARNING] " msg "\x1b[0m\n", ##__VA_ARGS__);

const int MEDIUM_INT_MAX = (1 << 23) - 1;
const int MEDIUM_INT_MIN = -(1 << 23);

enum VarType {
    PRIMITIVE,
    ARRAY
};

enum DataType {
    INT,
    CHAR,
    MEDIUM_INT,
    BOOLEAN
};

inline int getSize(DataType data_type) {
    if (data_type == INT) {
        return 4;
    } else if (data_type == MEDIUM_INT) {
        return 3;
    } else if (data_type == CHAR) {
        return 1;
    } else if (data_type == BOOLEAN) {
        return 1;
    } else {
        return -1;
    }
}

inline string getDataTypeStr(DataType data_type) {
    if (data_type == INT) {
        return "int";
    } else if (data_type == CHAR) {
        return "char";
    } else if (data_type == MEDIUM_INT) {
        return "medium_int";
    } else if (data_type == BOOLEAN) {
        return "boolean";
    } else {
        return "unknown";
    }
}

// A wrapper for the medium int data type
class medium_int {
   public:
    char data[3];

    medium_int() {
        data[0] = 0;
        data[1] = 0;
        data[2] = 0;
    }

    medium_int(int val) {
        if (val < MEDIUM_INT_MIN || val > MEDIUM_INT_MAX) {
            WARNING("Variable is of type medium int, value is out of range, compressing\n");
        }
        data[0] = val & 0xff;
        data[1] = (val >> 8) & 0xff;
        data[2] = (val >> 16) & 0xff;
    }

    int medIntToInt() {
        int val = 0;
        val |= data[0];
        val |= (data[1] << 8);
        val |= (data[2] << 16);
        int sign_bit = (val >> 23) & 1;
        if (sign_bit == 1) {
            val |= 0xff000000;
        }
        return val;
    }

    medium_int &operator=(int val) {
        if (val < MEDIUM_INT_MIN || val > MEDIUM_INT_MAX) {
            WARNING("Variable is of type medium int, value is out of range, compressing\n");
        }
        data[0] = val & 0xff;
        data[1] = (val >> 8) & 0xff;
        data[2] = (val >> 16) & 0xff;
        return *this;
    }
};

struct MyType {
    const int ind;
    const VarType var_type;
    const DataType data_type;
    const size_t len;

    MyType(int _ind, VarType _var_type, DataType _data_type, size_t _len) : ind(_ind), var_type(_var_type), data_type(_data_type), len(_len) {}

    void print() {
        printf("MyType: %d %s %s %zu\n", ind, getDataTypeStr(data_type).c_str(), var_type == PRIMITIVE ? "primitive" : "array", len);
    }
};

void createMem(size_t bytes, bool is_gc_Active = true, bool is_profiler_active = false, string file = "memory_footprint.txt");

MyType createVar(DataType type);
void assignVar(MyType &var, int val);
void assignVar(MyType &var, medium_int val);
void assignVar(MyType &var, char val);
void assignVar(MyType &var, bool val);
void readVar(MyType &var, void *ptr);

MyType createArr(DataType type, int len);
void assignArr(MyType &arr, int val[]);
void assignArr(MyType &arr, medium_int val[]);
void assignArr(MyType &arr, char val[]);
void assignArr(MyType &arr, bool val[]);
void assignArr(MyType &arr, int index, int val);
void assignArr(MyType &arr, int index, medium_int val);
void assignArr(MyType &arr, int index, char val);
void assignArr(MyType &arr, int index, bool val);
void readArr(MyType &arr, void *ptr);
void readArr(MyType &arr, int index, void *ptr);

void freeElem(MyType &var);
void gcActivate();

void initScope();
void endScope();

void cleanExit();

#endif