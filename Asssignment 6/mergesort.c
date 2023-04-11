#include "goodmalloc.h"

int main()
{
    srand(time(0));
    createMem();
    /* Start with the empty list */
    Node* head = NULL;
    // // Insert 6.  So linked list becomes 6->NULL
    for(int i = 0; i < 50000; i++)
    {
        pushList(&head, rand() % 100000 + 1);
    }
    printf("Created DLL is: ");
    printList(head);    
    endScope();
    return 0;
}