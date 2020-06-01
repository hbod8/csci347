/* A simple demonstration of a list in C
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

struct node
{
  int data;
  struct node *next;
};

struct queue
{
  struct node *head;
  struct node *tail;
};

void queue_init(struct queue *queue)
{
  queue->head = NULL;
  queue->tail = NULL;
}

bool queue_isEmpty(struct queue *queue)
{
  return queue->head == NULL;
}

void queue_insert(struct queue *queue, int value)
{
  struct node *tmp = malloc(sizeof(struct node));
  if (tmp == NULL)
  {
    fputs("malloc failed\n", stderr);
    exit(1);
  }

  /* create the node */
  tmp->data = value;
  tmp->next = NULL;

  if (queue->head == NULL)
  {
    queue->head = tmp;
  }
  else
  {
    queue->tail->next = tmp;
  }
  queue->tail = tmp;
}

int queue_remove(struct queue *queue)
{
  int retval = 0;
  struct node *tmp;

  if (!queue_isEmpty(queue))
  {
    tmp = queue->head;
    retval = tmp->data;
    queue->head = tmp->next;
    free(tmp);
  }
  return retval;
}

/*  Quick and dirty user interface ... */

int main()
{

  char cmd[255];

  struct queue myQueue;
  int val;

  queue_init(&myQueue);

  do
  {
    printf("cmd? ");
    if (fgets(cmd, 255, stdin) != cmd)
    {
      if (feof(stdin))
        exit(0);
      cmd[0] = '?';
    }

    switch (cmd[0])
    {
    case 'a':
      val = atoi(&cmd[1]);
      queue_insert(&myQueue, val);
      printf("Inserted %d into the queue\n", val);
      break;

    case 'd':
      val = queue_remove(&myQueue);
      printf("Removed %d from the queue\n", val);
      break;

    case 'l':
    {
      struct node *tmp = myQueue.head;
      while (tmp != NULL)
      {
        printf("    %d\n", tmp->data);
        tmp = tmp->next;
      }
    }
    break;

    case 'q':
      break;

    default:
      printf("Unknown command:  a->add, d->delete, l->list, q->quit\n");
    }

  } while (cmd[0] != 'q');

  return 0;
}
