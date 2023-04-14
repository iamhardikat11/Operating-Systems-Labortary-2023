#include "goodmalloc.hxx"
#include <iostream>
using namespace std;

// Utility function to swap two integers
void swap(int *A, int *B)
{
  int temp = *A;
  *A = *B;
  *B = temp;
}

// Split a doubly linked list (DLL) into 2 DLLs of half sizes
Node *split(Node *head)
{
  Node *fast = head, *slow = head;
  while (fast->next && fast->next->next)
  {
    fast = fast->next->next;
    slow = slow->next;
  }
  Node *temp = slow->next;
  slow->next = NULL;
  return temp;
}

Node *createNode(int data)
{
  Node *newNode = (Node *)malloc(sizeof(Node));
  newNode->data = data;
  newNode->next = NULL;
  newNode->prev = NULL;
  return newNode;
}

// Function to merge two linked lists
Node *merge(Node *first, Node *second)
{
  // If first linked list is empty
  if (!first)
    return second;

  // If second linked list is empty
  if (!second)
    return first;

  // Pick the smaller value
  if (first->data < second->data)
  {
    first->next = merge(first->next, second);
    first->next->prev = first;
    first->prev = NULL;
    return first;
  }
  else
  {
    second->next = merge(first, second->next);
    second->next->prev = second;
    second->prev = NULL;
    return second;
  }
}

// Function to do merge sort
Node *mergeSort(Node *head)
{
  if (!head || !head->next)
    return head;
  Node *second = split(head);

  // Recur for left and right halves
  head = mergeSort(head);
  second = mergeSort(second);
  // Merge the two sorted halves
  return merge(head, second);
}
int main()
{
    srand(time(0));
    createMem();
    int localAddress = createList("My_List", LL_INT, 50000);
    int arr[50000];
    for(int i = 0; i < 50000; i++)
        arr[i] = rand() % 100000 + 1;
    assignVal("My_List", 0, 50000, arr);
    DDL* head = (DDL *)data_->pageTable[localAddress/4];
    printList(head->list,"output_before.txt");
    head->list = mergeSort(head->list);
    printList(head->list, "output_after.txt");  
    endScope();
    return 0;
}
