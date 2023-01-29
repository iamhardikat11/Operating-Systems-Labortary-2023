#include "memlab.h"

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include <cstring>

using namespace std;

#ifdef LOGS
#define DEBUG(msg, ...) printf("\x1b[32m[DEBUG] " msg " \x1b[0m\n", ##__VA_ARGS__);
#define ERROR(msg, ...) printf("\x1b[31m[ERROR] " msg " \x1b[0m\n", ##__VA_ARGS__);
#define LIBRARY(msg, ...) printf("\x1b[34m[LIBRARY] " msg " \x1b[0m\n", ##__VA_ARGS__);
#define PAGE_TABLE(msg, ...) printf("\x1b[37m[PAGE TABLE] " msg " \x1b[0m\n", ##__VA_ARGS__);
#define WORD_ALIGN(msg, ...) printf("\x1b[36m[WORD ALIGNMENT] " msg " \x1b[0m\n", ##__VA_ARGS__);
#define GC(msg, ...) printf("\x1b[33m[GC] " msg "\x1b[0m\n", ##__VA_ARGS__);
#define STACK(msg, ...) printf("\x1b[37m[STACK] " msg "\x1b[0m\n", ##__VA_ARGS__);
#define MEMORY(msg, ...) printf("\x1b[37m[MEMORY] " msg "\x1b[0m\n", ##__VA_ARGS__);
#else
#define DEBUG(msg, ...)
#define ERROR(msg, ...)
#define LIBRARY(msg, ...)
#define PAGE_TABLE(msg, ...)
#define WORD_ALIGN(msg, ...)
#define GC(msg, ...)
#define STACK(msg, ...)
#define MEMORY(msg, ...)
#endif

typedef unsigned int u_int;
typedef long unsigned int u_long;

const size_t MAX_PT_ENTRIES = 1024;
const size_t MAX_STACK_SIZE = 1024;

const double EXTRA_MEM_FACTOR = 1.25;
const int GC_SLEEP_US = 10;
const double COMPACTION_RATIO_THRESHOLD = 3.0;

bool gc_active;
bool profiler_active;
FILE *fp;

void LOCK(pthread_mutex_t *mutex) {
    int status = pthread_mutex_lock(mutex);
    if (status != 0) {
        ERROR("pthread_mutex_lock failed: %s\n", strerror(status));
        exit(1);
    }
}

void UNLOCK(pthread_mutex_t *mutex) {
    int status = pthread_mutex_unlock(mutex);
    if (status != 0) {
        ERROR("pthread_mutex_unlock failed: %s\n", strerror(status));
        exit(1);
    }
}

// Reference: https://web2.qatar.cmu.edu/~msakr/15213-f09/lectures/class19.pdf
struct Memory {
    int *start;
    int *end;
    size_t size;  // in words
    size_t totalFree;
    u_int numFreeBlocks;
    size_t currMaxFree;
    pthread_mutex_t mutex;

    int init(size_t bytes) {
        bytes = ((bytes + 3) >> 2) << 2;
        start = (int *)malloc(bytes);
        if (start == NULL) {
            return -1;
        }
        end = start + (bytes >> 2);
        size = bytes >> 2;  // in words
        *start = (bytes >> 2) << 1;
        *(start + (bytes >> 2) - 1) = (bytes >> 2) << 1;

        totalFree = bytes >> 2;
        numFreeBlocks = 1;
        currMaxFree = bytes >> 2;

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
        pthread_mutex_init(&mutex, &attr);

        MEMORY("Memory segment created");
        MEMORY("start address = %p", start);
        MEMORY("size = %lu (in words)", size);
        return 0;
    }

    // Absolute address to offset
    int getOffset(int *p) {
        return (int)(p - start);
    }

    // Offset to absolute address
    int *getAddr(int offset) {
        return (start + offset);
    }

    // Finds a free block of memory for sz words
    int *findFreeBlock(size_t sz) {  // sz is the size required for the data (in words)
        MEMORY("Finding free block for %lu word(s) of data", sz);
        int *p = start;
        while ((p < end) && ((*p & 1) || ((size_t)(*p >> 1) < sz + 2))) {  // keep on iterating till a suitable block is found
            p = p + (*p >> 1);
        }
        if (p < end) {
            MEMORY("Found free block at %p", p);
            return p;
        } else {
            MEMORY("No free block found");
            return NULL;
        }
    }

    // Allocates memory for sz words at address p and sets the appropriate headers and footers
    void allocateBlock(int *p, size_t sz) {  // sz is the size required for the data (in words)
        sz += 2;
        u_int old_size = *p >> 1;       // mask out low bit
        *p = (sz << 1) | 1;             // set new length and allocated bit for header
        *(p + sz - 1) = (sz << 1) | 1;  // same for footer

        if (sz < old_size) {
            *(p + sz) = (old_size - sz) << 1;            // set length in remaining for header
            *(p + old_size - 1) = (old_size - sz) << 1;  // same for footer
        }

        totalFree -= sz;
        if (sz == old_size) {
            numFreeBlocks--;
        }
        if (old_size == currMaxFree) {
            currMaxFree -= sz;
        }
        currMaxFree = max(currMaxFree, totalFree / (numFreeBlocks + 1));
        if (profiler_active) {
            fprintf(fp, "%ld\n", size - totalFree);
        }
        MEMORY("Allocated block at %p for %lu word(s) of data", p, sz - 2);
    }

    // Deallocates the memory block at address p and sets the appropriate headers and footers
    void freeBlock(int *p) {
        MEMORY("Freeing block at %p", p);
        *p = *p & -2;  // clear allocated flag in header
        u_int curr_size = *p >> 1;
        *(p + curr_size - 1) = *(p + curr_size - 1) & -2;  // clear allocated flag in footer

        totalFree += curr_size;
        numFreeBlocks++;

        int *next = p + curr_size;                // find next block
        if ((next != end) && (*next & 1) == 0) {  // if next block is free
            MEMORY("Coalescing with next block at %p", next);
            u_int next_size = *next >> 1;
            *p = (curr_size + next_size) << 1;                                // merge with next block
            *(p + curr_size + next_size - 1) = (curr_size + next_size) << 1;  // set length in footer
            numFreeBlocks--;
            curr_size += next_size;
        }

        if ((p != start) && (*(p - 1) & 1) == 0) {  // if previous block is free
            u_int prev_size = *(p - 1) >> 1;
            MEMORY("Coalescing with previous block at %p", (p - prev_size));
            *(p - prev_size) = (prev_size + curr_size) << 1;      // set length in header of prev
            *(p + curr_size - 1) = (prev_size + curr_size) << 1;  // set length in footer
            numFreeBlocks--;
            curr_size += prev_size;
        }

        currMaxFree = max(currMaxFree, (size_t)curr_size);
        currMaxFree = max(currMaxFree, totalFree / (numFreeBlocks + 1));
        if (profiler_active) {
            fprintf(fp, "%ld\n", size - totalFree);
        }
    }

    // Display the current memory blocks
    void displayMem() {
#ifdef LOGS
        int *p = start;
        printf("   Start      End    Allocated\n");
        while (p < end) {
            printf("%7ld %9ld %10d\n", p - start, (p - start - 1) + (*p >> 1), *p & 1);
            p = p + (*p >> 1);
        }
        printf("Total free memory = %ld words, Largest free block = %ld words, No. of free blocks = %d\n", totalFree, currMaxFree, numFreeBlocks);
#endif
    }
};

// Counter to index in page table array
u_int counterToIdx(u_int p) {
    return (p >> 2);
}

// Index in page table array to counter
u_int idxToCounter(u_int p) {
    return (p << 2);
}

struct PageTableEntry {
    u_int addr : 30;
    u_int valid : 1;
    u_int marked : 1;

    void init() {
        addr = 0;
        valid = 0;
        marked = 0;
    }

    void print() {
        printf("%10d %6d %6d\n", addr, valid, marked);
    }
};

struct PageTable {
    PageTableEntry pt[MAX_PT_ENTRIES];
    u_int head, tail;
    size_t size;
    pthread_mutex_t mutex;

    void init() {
        for (size_t i = 0; i < MAX_PT_ENTRIES; i++) {
            pt[i].init();
            pt[i].addr = i + 1;
        }
        head = 0;
        tail = MAX_PT_ENTRIES - 1;
        size = 0;
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
        pthread_mutex_init(&mutex, &attr);
        PAGE_TABLE("Page table initialized");
    }

    // Adds a new entry to the page table
    int insert(u_int addr) {
        if (size == MAX_PT_ENTRIES) {
            PAGE_TABLE("Page table is full, insert failed");
            return -1;
        }
        u_int idx = head;
        u_int next = pt[head].addr;
        pt[idx].addr = addr;
        pt[idx].valid = 1;
        pt[idx].marked = 1;
        size++;
        if (size < MAX_PT_ENTRIES) {
            head = next;
        } else {
            head = MAX_PT_ENTRIES;
            tail = MAX_PT_ENTRIES;
        }
        PAGE_TABLE("Inserted new page table entry with memory offset %d at array index %d", addr, idx);
        return idx;
    }

    // Removes an entry from the page table
    int remove(u_int idx) {
        if (size == 0 || !pt[idx].valid) {
            PAGE_TABLE("Page table is empty or entry index %d is invalid, remove failed", idx);
            return -1;
        }
        pt[idx].valid = 0;
        pt[tail].addr = idx;
        tail = idx;
        size--;
        if (size == MAX_PT_ENTRIES - 1) {
            head = tail;
        }
        PAGE_TABLE("Removed entry with array index %d in the page table", idx);
        return 0;
    }

    // Display the contents of the page table
    void print() {
        printf("\nPage Table:\n");
        printf("Head: %d, Tail: %d, Size: %lu\n", head, tail, size);
        printf("Index     Entry  Valid  Marked\n");
        for (size_t i = 0; i < MAX_PT_ENTRIES; i++) {
            if (pt[i].valid) {
                printf("%3ld ", i);
                pt[i].print();
            }
        }
    }
};

struct Stack {
    int st[MAX_STACK_SIZE];
    size_t size;

    void init() {
        size = 0;
        STACK("Stack initialized");
    }

    int top() {
        if (size == 0) {
            return -2;
        }
        return st[size - 1];
    }

    int push(int v) {
        if (size == MAX_STACK_SIZE) {
            STACK("Stack is full, push failed");
            return -2;
        }
        st[size++] = v;
        STACK("Pushed %d onto stack", v);
        return 0;
    }

    int pop() {
        if (size == 0) {
            STACK("Stack is empty, pop failed");
            return -2;
        }
        int v = st[--size];
        STACK("Popped %d from stack", v);
        return v;
    }

    void print() {
        printf("\nGlobal Variable Stack:\n");
        printf("Size: %lu\n", size);
        for (size_t i = 0; i < size; i++) {
            printf("%d ", st[i]);
        }
        printf("\n");
    }
};

Memory *mem;
PageTable *page_table;
Stack *var_stack;
pthread_t gc_tid;
sem_t gc_sem;

void freeElem(u_int idx) {
    GC("freeElem called for array index %d in page table", idx);
    int ret = page_table->remove(idx);  // Remove the entry from the page table
    if (ret == -1) {
        throw runtime_error("freeElem: Invalid Index");
    }
    mem->freeBlock(mem->getAddr(page_table->pt[idx].addr));  // Free the memory block
}

void freeElem(MyType &var) {
    LIBRARY("freeElem called for variable with counter = %d", var.ind);
    LOCK(&mem->mutex);
    LOCK(&page_table->mutex);
    if (page_table->pt[counterToIdx(var.ind)].valid) {
        freeElem(counterToIdx(var.ind));
    }
    UNLOCK(&page_table->mutex);
    UNLOCK(&mem->mutex);
}

// Calculates new offsets that will be used for compaction
void calcNewOffsets() {
    int *p = mem->start;
    u_int free = 0;
    while (p < mem->end) {
        if ((*p & 1) == 0) {
            free += (*p >> 1);
        } else {
            *(p + (*p >> 1) - 1) = (((p - free) - mem->start) << 1) | 1;
        }
        p = p + (*p >> 1);
    }
    GC("Completed calculating new offsets for blocks for compaction");
}

// Updates the page table entries with the new offsets for compaction
void updatePageTable() {
    for (size_t i = 0; i < MAX_PT_ENTRIES; i++) {
        if (page_table->pt[i].valid) {
            int *p = mem->getAddr(page_table->pt[i].addr);
            int newAddr = *(p + (*p >> 1) - 1) >> 1;
            PAGE_TABLE("Index: %ld, Old addr: %d, New addr: %d", i, page_table->pt[i].addr, newAddr);
            page_table->pt[i].addr = newAddr;
        }
    }
    GC("Page table updated for compaction");
}

void compactMemory() {
    GC("Before compaction:");
    mem->displayMem();
    GC("Starting memory compaction");
    calcNewOffsets();
    updatePageTable();
    int *p = mem->start;
    int *next = p + (*p >> 1);
    while (next < mem->end) {
        if ((*p & 1) == 0 && (*next & 1) == 1) {  // If curent block is free and next block is allocated, swap them
            int curr_size = *p >> 1;
            int next_size = *next >> 1;
            memcpy(p, next, next_size << 2);
            p = p + next_size;
            *p = curr_size << 1;
            *(p + curr_size - 1) = curr_size << 1;
            next = p + curr_size;
            if (next < mem->end && (*next & 1) == 0) {  // Coalesce if next block is free
                curr_size = curr_size + (*next >> 1);
                *p = curr_size << 1;
                *(p + curr_size - 1) = curr_size << 1;
                next = p + curr_size;
            }
        } else {
            p = next;
            next = p + (*p >> 1);
        }
    }
    GC("Blocks rearranged after compaction");
    p = mem->start;
    while (p < mem->end) {  // Update block footers
        *(p + (*p >> 1) - 1) = *p;
        p = p + (*p >> 1);
    }
    GC("Block footers updated");
    mem->numFreeBlocks = 1;
    mem->currMaxFree = mem->totalFree;
    GC("Memory compaction completed");
    GC("After compaction:");
    mem->displayMem();
}

void gcRun() {
    LOCK(&mem->mutex);
    LOCK(&page_table->mutex);
    GC("gcRun called");
    // Perform mark and sweep
    for (size_t i = 0; i < MAX_PT_ENTRIES; i++) {
        if (page_table->pt[i].valid && !page_table->pt[i].marked) {
            freeElem(i);
        }
    }
    // Check if compaction needs to be done
    double ratio = (double)mem->totalFree / (double)(mem->currMaxFree + 1);
    GC("Ratio (Total Free/Largest Free) = %f", ratio);
    if (ratio >= COMPACTION_RATIO_THRESHOLD) {
        GC("Ratio more than compaction ratio threshold");
        compactMemory();
    }
    GC("gcRun finished");
    UNLOCK(&page_table->mutex);
    UNLOCK(&mem->mutex);
}

void gcActivate() {
    GC("gcActivate called");
    if (gc_active) {
        pthread_kill(gc_tid, SIGUSR1);
    }
}

// Signal handler for SIGUSR1
void handleSIGUSR1(int sig) {
    GC("SIGUSR1 received");
    gcRun();
}

// The function that is run by the garbage collection thread
void *gcThread(void *arg) {
    GC("Garbage collection thread created");
    signal(SIGUSR1, handleSIGUSR1);
    GC("Signal handler for SIGUSR1 registered");
    sem_post(&gc_sem);
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
    // SIGUSR1 is blocked while gcRun is executing, and unblocked when it is over
    while (1) {
        pthread_testcancel();
        usleep(GC_SLEEP_US);
        pthread_sigmask(SIG_BLOCK, &mask, NULL);
        GC("Garbage collection thread awakened from sleep");
        pthread_testcancel();
        gcRun();
        pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
    }
}

// Indicates that a new scope has been entered
void initScope() {
    LIBRARY("initScope called");
    if (gc_active) {
        if (var_stack->push(-1) < 0) {
            throw runtime_error("initScope: Stack full, cannot push");
        }
    }
}

// Indicates that the current scope has ended
void endScope() {
    LIBRARY("endScope called");
    if (gc_active) {
        int ind;
        do {
            ind = var_stack->pop();
            if (ind == -2) {
                throw runtime_error("endScope: Stack empty, cannot pop");
            }
            if (ind >= 0) {
                LOCK(&page_table->mutex);
                page_table->pt[counterToIdx(ind)].marked = 0;  // Set mark bit to 0
                PAGE_TABLE("Unmarked entry in page table for variable with counter = %d", ind);
                UNLOCK(&page_table->mutex);
            }
        } while (ind >= 0);
    }
}

// Function to exit by freeing up all resources
void cleanExit() {
    LIBRARY("cleanExit called");
    LOCK(&mem->mutex);
    LOCK(&page_table->mutex);
    if (gc_active) {
        pthread_cancel(gc_tid);
    }
    sem_destroy(&gc_sem);
    pthread_mutex_destroy(&mem->mutex);
    pthread_mutex_destroy(&page_table->mutex);
    free(var_stack);
    STACK("Freed memory allotted to stack");
    free(page_table);
    PAGE_TABLE("Freed memory allotted to page table");
    free(mem->start);
    free(mem);
    MEMORY("Freed main memory");
    exit(0);
}

// Get word location for arrays using index in the array
int idxToWord(DataType type, int idx) {
    int cnt;
    if (type == BOOLEAN) {
        cnt = (1 << 5) / getSize(type);
    } else {
        cnt = (1 << 2) / getSize(type);
    }
    int word = idx / cnt;
    return word;
}

// Get offset in a word for arrays using index in the array
int idxToOffset(DataType type, int idx) {
    int cnt;
    if (type == BOOLEAN) {
        cnt = (1 << 5) / getSize(type);
    } else {
        cnt = (1 << 2) / getSize(type);
    }
    int word = idx / cnt;
    return (idx - word * cnt) * getSize(type);
}

void createMem(size_t bytes, bool is_gc_active, bool is_profiler_active, string file) {
    LIBRARY("createMem called");
    if (mem != NULL) {
        throw runtime_error("createMem: Memory already created");
    }
    bytes = (size_t)(bytes * EXTRA_MEM_FACTOR);
    bytes = ((bytes + 3) >> 2) << 2;
    mem = (Memory *)malloc(sizeof(Memory));
    if (mem->init(bytes) == -1) {
        throw runtime_error("createMem: Memory allocation failed");
    }

    page_table = (PageTable *)malloc(sizeof(PageTable));
    page_table->init();

    var_stack = (Stack *)malloc(sizeof(Stack));
    var_stack->init();

    gc_active = is_gc_active;  // To switch on/off garbage collection
    profiler_active = is_profiler_active;

    if (profiler_active) {  // For checking impact of garbage collection
        fp = fopen(file.c_str(), "w");
    }

    sem_init(&gc_sem, 0, 0);
    if (gc_active) {
        pthread_create(&gc_tid, NULL, gcThread, NULL);
        sem_wait(&gc_sem);  // Wait till the signal handler is installed in the garbage collection thread
    }
}

MyType create(VarType var_type, DataType data_type, u_int len, u_int size_req) {
    LOCK(&mem->mutex);
    int *p = mem->findFreeBlock(size_req);
    if (p == NULL) {
        LOCK(&page_table->mutex);
        MEMORY("Could not find free block, trying compaction");
        compactMemory();
        UNLOCK(&page_table->mutex);
        p = mem->findFreeBlock(size_req);
        if (p == NULL) {
            throw runtime_error("create: No free block in memory");
        }
    }
    mem->allocateBlock(p, size_req);
    int addr = mem->getOffset(p);
    LOCK(&page_table->mutex);
    int idx = page_table->insert(addr);
    if (idx < 0) {
        throw runtime_error("create: No free space in page table");
    }
    UNLOCK(&page_table->mutex);
    UNLOCK(&mem->mutex);
    u_int ind = idxToCounter(idx);
    if (var_stack->push(ind) < 0) {
        throw runtime_error("create: Stack full, cannot push");
    }
    return MyType(ind, var_type, data_type, len);
}

MyType createVar(DataType type) {
    LIBRARY("createVar called with type = %s", getDataTypeStr(type).c_str());
    WORD_ALIGN("Creating variable, so memory required = 1 word");
    return create(PRIMITIVE, type, 1, 1);
}

// Type checking
void validate(MyType &var, VarType type, DataType d_type, bool index = false) {
    string vt = (type == PRIMITIVE ? "Var" : "Arr");
    string s = "";
    if (type == ARRAY) {
        if (!index) {
            s = "[]";
        } else {
            s = "[], index";
        }
    }
    string func = "assign" + vt + " (" + getDataTypeStr(d_type) + s + "): ";
    if (var.var_type != type) {
        string str = (type == PRIMITIVE ? "primitive" : "array");
        throw runtime_error(func + "Variable is not a " + str);
    }
    if (var.data_type != d_type) {
        throw runtime_error(func + "Type mismatch. Data type of variable is " + getDataTypeStr(var.data_type));
    }
    if (!page_table->pt[counterToIdx(var.ind)].valid) {
        throw runtime_error(func + "Variable is not valid");
    }
}

// Assign an int
void assignVar(MyType &var, int val) {
    LIBRARY("assignVar (int) called for variable with counter = %d and value = %d", var.ind, val);
    validate(var, PRIMITIVE, INT);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(var.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    memcpy(p, &val, 4);
    WORD_ALIGN("Data type = %s, wrote 1 word to memory", getDataTypeStr(var.data_type).c_str());
    UNLOCK(&mem->mutex);
}

// Assign a medium int
void assignVar(MyType &var, medium_int val) {
    LIBRARY("assignVar (medium int) called for variable with counter = %d and value = %d", var.ind, val.medIntToInt());
    validate(var, PRIMITIVE, MEDIUM_INT);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(var.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    int temp = val.medIntToInt();
    memcpy(p, &temp, 4);
    WORD_ALIGN("Data type = %s, wrote 1 word to memory", getDataTypeStr(var.data_type).c_str());
    UNLOCK(&mem->mutex);
}

// Assign a char
void assignVar(MyType &var, char val) {
    LIBRARY("assignVar (char) called for variable with counter = %d and value = %c", var.ind, val);
    validate(var, PRIMITIVE, CHAR);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(var.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    int temp = (int)val;
    memcpy(p, &temp, 4);
    WORD_ALIGN("Data type = char, wrote 1 word (1 byte data + 3 byte padding) to memory");
    UNLOCK(&mem->mutex);
}

// Assign a boolean
void assignVar(MyType &var, bool val) {
    LIBRARY("assignVar (bool) called for variable with counter = %d and value = %d", var.ind, val);
    validate(var, PRIMITIVE, BOOLEAN);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(var.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    int temp = (int)val;
    memcpy(p, &temp, 4);
    WORD_ALIGN("Data type = boolean, wrote 1 word (1 bit data + 3 byte, 7 bit padding) to memory");
    UNLOCK(&mem->mutex);
}

// Reads the value of a variable and stores it in the memory location pointed to by ptr
void readVar(MyType &var, void *ptr) {
    LIBRARY("readVar called for variable with counter = %d", var.ind);
    if (var.var_type != PRIMITIVE) {
        throw runtime_error("readVar: Variable is not a primitive");
    }
    if (!page_table->pt[counterToIdx(var.ind)].valid) {
        throw runtime_error("readVar: Variable is not valid");
    }
    int size = getSize(var.data_type);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(var.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    int t = *(int *)p;
    WORD_ALIGN("Extracted entire 1 word from memory");
    if (var.data_type == MEDIUM_INT) {
        medium_int temp(t);
        memcpy(ptr, &temp, size);
    } else {
        memcpy(ptr, &t, size);
    }
    WORD_ALIGN("Wrote %d bytes to the destination address", size);
    UNLOCK(&mem->mutex);
}

MyType createArr(DataType type, int len) {
    LIBRARY("createArr called with type = %s and len = %d", getDataTypeStr(type).c_str(), len);
    if (len <= 0) {
        throw runtime_error("createArr: Length of array should be greater than 0");
    }
    u_int size_req;
    if (type == INT || type == MEDIUM_INT) {
        size_req = len;  // 1 int/medium_int in 1 word
    } else if (type == CHAR) {
        size_req = (len + 3) >> 2;  // 4 chars in one word
    } else if (type == BOOLEAN) {
        size_req = (len + 31) >> 5;  // 32 booleans in one word
    }
    WORD_ALIGN("Creating array of type = %s, len = %d, memory required = %d words", getDataTypeStr(type).c_str(), len, size_req);
    return create(ARRAY, type, len, size_req);
}

// Assign an entire array of ints
void assignArr(MyType &arr, int val[]) {
    LIBRARY("assignArr (int) called for array with counter = %d", arr.ind);
    validate(arr, ARRAY, INT);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    WORD_ALIGN("Data type = %s, writing 1 word chunks to memory", getDataTypeStr(arr.data_type).c_str());
    for (size_t i = 0; i < arr.len; i++) {
        memcpy(p + i, &val[i], 4);
    }
    UNLOCK(&mem->mutex);
}

// Assign an entire array of medium ints
void assignArr(MyType &arr, medium_int val[]) {
    LIBRARY("assignArr (medium int) called for array with counter = %d", arr.ind);
    validate(arr, ARRAY, MEDIUM_INT);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    WORD_ALIGN("Data type = %s, writing 1 word chunks to memory", getDataTypeStr(arr.data_type).c_str());
    for (size_t i = 0; i < arr.len; i++) {
        int temp = val[i].medIntToInt();
        memcpy(p + i, &temp, 4);
    }
    UNLOCK(&mem->mutex);
}

// Assign an entire array of chars
void assignArr(MyType &arr, char val[]) {
    LIBRARY("assignArr (char) called for array with counter = %d", arr.ind);
    validate(arr, ARRAY, CHAR);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    WORD_ALIGN("Data type = char, writing 4 array elements into 1 word in memory");
    for (size_t i = 0; i < arr.len; i += 4) {
        u_int temp = 0;
        for (size_t j = 0; j < 4; j++) {
            char c = (i + j < arr.len) ? val[i + j] : 0;
            temp = temp | ((u_int)c << (j * 8));
        }
        memcpy((char *)p + i, &temp, 4);
    }
    UNLOCK(&mem->mutex);
}

// Assign an entire array of booleans
void assignArr(MyType &arr, bool val[]) {
    LIBRARY("assignArr (bool) called for array with counter = %d", arr.ind);
    validate(arr, ARRAY, BOOLEAN);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    WORD_ALIGN("Data type = boolean, writing 32 array elements into 1 word in memory");
    for (size_t i = 0; i < arr.len; i += 4) {
        u_int temp = 0;
        for (size_t j = 0; j < 32; j++) {
            bool c = (i * 8 + j < arr.len) ? val[i * 8 + j] : false;
            temp = temp | ((u_int)c << j);
        }
        memcpy((char *)p + i, &temp, 4);
    }
    UNLOCK(&mem->mutex);
}

// Assign an element at a index in an array of ints
void assignArr(MyType &arr, int index, int val) {
    LIBRARY("assignArr (int[], index) called for array with counter = %d at index = %d and value = %d", arr.ind, index, val);
    validate(arr, ARRAY, INT, true);
    if (index < 0 || index >= (int)arr.len) {
        throw runtime_error("assignArr (int[], index): Index out of range");
    }
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    WORD_ALIGN("Data type = %s, reading 1 word from memory", getDataTypeStr(arr.data_type).c_str());
    memcpy(p + index, &val, 4);
    UNLOCK(&mem->mutex);
}

// Assign an element at a index in an array of medium ints
void assignArr(MyType &arr, int index, medium_int val) {
    LIBRARY("assignArr (medium int[], index) called for array with counter = %d at index = %d and value = %d", arr.ind, index, val.medIntToInt());
    validate(arr, ARRAY, MEDIUM_INT, true);
    if (index < 0 || index >= (int)arr.len) {
        throw runtime_error("assignArr (medium int[], index): Index out of range");
    }
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    WORD_ALIGN("Data type = %s, reading 1 word from memory", getDataTypeStr(arr.data_type).c_str());
    int temp = val.medIntToInt();
    memcpy(p + index, &temp, 4);
    UNLOCK(&mem->mutex);
}

// Assign an element at a index in an array of chars
void assignArr(MyType &arr, int index, char val) {
    LIBRARY("assignArr (char[], index) called for array with counter = %d at index = %d and value = %c", arr.ind, index, val);
    validate(arr, ARRAY, CHAR, true);
    if (index < 0 || index >= (int)arr.len) {
        throw runtime_error("assignArr (char[], index): Index out of range");
    }
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    int *q = p + idxToWord(arr.data_type, index);
    int offset = idxToOffset(arr.data_type, index);
    WORD_ALIGN("Data type = char, reading entire 1 word memory");
    int temp = *q;
    WORD_ALIGN("Changing byte no. %d in the word", offset);
    temp = temp & ~(0xff << (offset * 8));
    temp = temp | (val << (offset * 8));
    WORD_ALIGN("Writing the word back to memory");
    memcpy(q, &temp, 4);
    UNLOCK(&mem->mutex);
}

// Assign an element at a index in an array of booleans
void assignArr(MyType &arr, int index, bool val) {
    LIBRARY("assignArr (bool[], index) called for array with counter = %d at index = %d and value = %d", arr.ind, index, val);
    validate(arr, ARRAY, BOOLEAN, true);
    if (index < 0 || index >= (int)arr.len) {
        throw runtime_error("assignArr (bool[], index): Index out of range");
    }
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    int *q = p + idxToWord(arr.data_type, index);
    int offset = idxToOffset(arr.data_type, index);
    WORD_ALIGN("Data type = char, reading entire 1 word memory");
    int temp = *q;
    WORD_ALIGN("Changing bit no. %d in the word", offset);
    temp = temp & ~(1 << offset);
    temp = temp | (val << offset);
    WORD_ALIGN("Writing the word back to memory");
    memcpy(q, &temp, 4);
    UNLOCK(&mem->mutex);
}

// Reads the entire array and stores it in the memory location pointed to by ptr
void readArr(MyType &arr, void *ptr) {
    LIBRARY("readArr called for array with counter = %d", arr.ind);
    if (arr.var_type != ARRAY) {
        throw runtime_error("readArr: Variable is not a array");
    }
    if (!page_table->pt[counterToIdx(arr.ind)].valid) {
        throw runtime_error("readArr: Variable is not valid");
    }
    int size = getSize(arr.data_type);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    if (arr.data_type == INT) {
        WORD_ALIGN("Data type = int, copying 1 word chunks from memory to the destination address");
        for (size_t i = 0; i < arr.len; i++) {
            memcpy((int *)ptr + i, p + i, size);
        }
    } else if (arr.data_type == MEDIUM_INT) {
        WORD_ALIGN("Data type = medium int, copying 1 word chunks from memory to the destination address");
        for (size_t i = 0; i < arr.len; i++) {
            int temp = *(p + i);
            medium_int t(temp);
            memcpy((medium_int *)ptr + i, &t, size);
        }
    } else if (arr.data_type == CHAR) {
        WORD_ALIGN("Data type = char, copying 1 word chunks (= 4 array elements) from memory to the destination address");
        size_t blocks = (arr.len + 3) >> 2;
        for (size_t i = 0; i < blocks; i++) {
            u_int temp = *(p + i);
            for (size_t j = 0; j < 4; j++) {
                if (i * 4 + j < arr.len) {
                    memcpy((char *)ptr + (i * 4 + j), &temp, 1);
                    temp = temp >> 8;
                }
            }
        }
    } else if (arr.data_type == BOOLEAN) {
        WORD_ALIGN("Data type = boolean, copying 1 word chunks (= 32 array elements) from memory to the destination address");
        size_t blocks = (((arr.len + 31) >> 5) << 5) >> 4;
        for (size_t i = 0; i < blocks; i++) {
            u_int temp = *(p + i);
            for (size_t j = 0; j < 32; j++) {
                if (i * 32 + j < arr.len) {
                    *((bool *)ptr + i * 32 + j) = (temp & (1 << j)) != 0;
                }
            }
        }
    }
    UNLOCK(&mem->mutex);
}

// Reads the value of a single array element and stores it in the memory location pointed to by ptr
void readArr(MyType &arr, int index, void *ptr) {
    LIBRARY("readArr (index) called for array with counter = %d at index = %d", arr.ind, index);
    if (arr.var_type != ARRAY) {
        throw runtime_error("readArr (index): Variable is not a array");
    }
    if (!page_table->pt[counterToIdx(arr.ind)].valid) {
        throw runtime_error("readArr (index): Variable is not valid");
    }
    if (index < 0 || index >= (int)arr.len) {
        throw runtime_error("readArr (index): Index out of range");
    }
    int size = getSize(arr.data_type);
    LOCK(&mem->mutex);
    u_int idx = counterToIdx(arr.ind);
    int *p = mem->getAddr(page_table->pt[idx].addr) + 1;
    int *q = p + idxToWord(arr.data_type, index);
    int offset = idxToOffset(arr.data_type, index);
    int t = *q;
    WORD_ALIGN("Read 1 word from memory");
    if (arr.data_type == MEDIUM_INT) {
        medium_int temp(t);
        memcpy(ptr, &temp, size);
        WORD_ALIGN("Data type = %s, 3 bytes copied to destination address", getDataTypeStr(arr.data_type).c_str());
    }
    if (arr.data_type == BOOLEAN) {
        t = (t >> offset) & 1;
        *(bool *)ptr = t;
        WORD_ALIGN("Bit no. %d written to destination address for boolean", offset);
    } else {
        memcpy(ptr, (char *)(&t) + offset, size);
        if (arr.data_type == CHAR) {
            WORD_ALIGN("Data type = char, byte no. %d copied to destination address", offset);
        } else {
            WORD_ALIGN("Data type = %s, 1 word copied to destination address", getDataTypeStr(arr.data_type).c_str());
        }
    }
    UNLOCK(&mem->mutex);
}
