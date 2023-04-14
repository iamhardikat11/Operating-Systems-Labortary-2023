
// C program for merge sort on doubly linked list
#include<stdio.h>
#include<stdlib.h>
struct Node
{
    int data;
    struct Node *next, *prev;
};
// A utility function to insert a new node at the
// beginning of doubly linked list
void insert(struct Node **head, int data)
{
    struct Node *temp =
        (struct Node *)malloc(sizeof(struct Node));
    temp->data = data;
    temp->next = temp->prev = NULL;
    if (!(*head))
        (*head) = temp;
    else
    {
        temp->next = *head;
        (*head)->prev = temp;
        (*head) = temp;
    }
}