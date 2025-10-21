#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct node {
  struct node *next;
  int data;
};

struct node *reverse(struct node *head);

int main() {
  // Create list 4->3->2->1->0->NULL
  struct node *head = NULL;

  for (int i = 0; i < 5; i++) {
    struct node *new_head = malloc(sizeof(struct node));
    new_head->next = head;
    new_head->data = i;
    
    head = new_head;
  }

  // Reverse list
  struct node *reversed = reverse(head);

  // Print elements in list
  while (reversed != NULL) {
    printf("%d\n", (__int64_t) reversed);
    printf("%d\n", reversed->data);
    
    struct node *next = reversed->next;
    free(reversed);
    reversed = next;
  }

  return 0;
}

//
// Place the list pointed to by head into reverse order.
// The head of the list is returned.
//
struct node *reverse(struct node *head) {
  struct node *previous = NULL;

  if (head == NULL) {
    return head;
  }

  while (head != NULL) {
    struct node *next = head->next;
    head->next = previous;
    previous = head;
    head = next;
  }

  return previous;
}
