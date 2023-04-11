#include "goodmalloc.h"

int main()
{
    srand(time(0));
    createMem();

    /* Start with the empty list */
    Node* head = NULL;
  
    // // Insert 6.  So linked list becomes 6->NULL
    // append(&head, 6);
  
    // // Insert 7 at the beginning. So linked list becomes
    // // 7->6->NULL
    // pushList(&head, 7);
  
    // // Insert 1 at the beginning. So linked list becomes
    // // 1->7->6->NULL
    // pushList(&head, 1);
  
    // // Insert 4 at the end. So linked list becomes
    // // 1->7->6->4->NULL
    // appendList(&head, 4);
  
    // // Insert 8, after 7. So linked list becomes
    // // 1->7->8->6->4->NULL
    // insertAfterList(head->next, 8);
  
    // printf("Created DLL is: ");
    // printList(head);
  
    getchar();
    endScope();
    return 0;
}