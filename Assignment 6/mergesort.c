#include "goodmalloc.h"

int main()
{
    srand(time(0));
    createMem();
    
    int localAddress = createList("My_List", LL_INT, 50000);
    // mediumInt *physicalAddress = (mediumInt *)data_->pageTable[localAddr / 4];
    DDL* head = (DDL *)data_->pageTable[localAddress/4];
    printList(head->list, "out_before.txt");
    head->list = mergeSort(head->list);
    printList(head->list, "out_after.txt");  
    endScope();
    return 0;
}
