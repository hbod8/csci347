#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

#define KAYAK 1
#define CANOE 2
#define SAILBOAT 4

struct group
{
  long number;
  int lifejackets;
};

/* Attributes */
/* Number of threads. */
int threadc;
/* Rate at which groups arrive to Lakewood */
int grouprate;

/* Fatal error handler */
void fatal(long n)
{
  fprintf(stderr, "Fatal error on thread %ld.\n", n);
  exit(n);
}

/* Queue */
/* Queue mutex */
pthread_mutex_t queuemutex;

/* Queue node */
struct node {
  long data;
  struct node *next;
};

/* Queue structure */
struct queue {
  struct node *head;
  struct node *tail;
  int size;
};

/* Initalize queue */
void queue_init(struct queue *queue)
{
  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;
}

/* Queue empty boolean condition */
bool queue_isEmpty(struct queue *queue)
{
  return queue->head == NULL;
}

/* Queue insert */
void queue_insert(struct queue *queue, long value)
{
  struct node *tmp = malloc(sizeof(struct node));
  if (tmp == NULL)
  {
    fprintf(stderr, "malloc failed");
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
  queue->size++;
}

/* Queue pop */
long queue_pop(struct queue *queue)
{
  long retval = 0;
  struct node *tmp;

  if (!queue_isEmpty(queue))
  {
    tmp = queue->head;
    retval = tmp->data;
    queue->head = tmp->next;
    free(tmp);
    queue->size--;
  }
  return retval;
}

/* Monitor */
/* Get lifejackets */

/* Convert lifejacket number to string of watercraft type */
char *gettype(int t)
{
  if (t == KAYAK)
  {
    return "kayak";
  }
  else if (t == CANOE)
  {
    return "canoe";
  }
  else if (t == SAILBOAT)
  {
    return "sailboat";
  }
  return "";
}

/* Print thread info */
void printgroup(struct group *g)
{
  printf("Group %ld wants to rent a %s and needs %d lifejackets.\n", g->number, gettype(g->lifejackets), g->lifejackets);
}

/* Thread Entry Point */
void *thread_body(void *arg)
{
  struct group *this = (struct group *)arg;
  printgroup(this);

  return arg;
}

/* Entry Point */
int main(int argc, char **args)
{
  /* Process command line arguments */
  time_t t;
  if (argc == 2)
  {
    threadc = atoi(args[1]);
    grouprate = 6;
    srand((unsigned)time(&t));
  }
  else if (argc == 3)
  {
    threadc = atoi(args[1]);
    grouprate = atoi(args[2]);
    srand((unsigned)time(&t));
  }
  else if (argc == 4)
  {
    threadc = atoi(args[1]);
    grouprate = atoi(args[2]);
    srand(0);
  }
  else
  {
    fprintf(stderr, "Usage: %s groups [rate] / [rate norand]\n", args[0]);
    exit(1);
  }
  /* Create renter information */
  struct group groups[threadc];

  for (int i = 0; i < threadc; i++)
  {
    groups[i].number = i;
    int r = (rand() % 3);
    if (r == 0)
    {
      groups[i].lifejackets = KAYAK;
    }
    else if (r == 1)
    {
      groups[i].lifejackets = CANOE;
    }
    else if (r == 2)
    {
      groups[i].lifejackets = SAILBOAT;
    }
  }

  /* Create threads */
  pthread_t threads[threadc];

  for (int i = 0; i < threadc; i++)
  {
    if (pthread_create(&threads[i], NULL, thread_body, (void *)&groups[i]))
    {
      fatal(i);
    }
  }

  /* Join threads */
  void *returnval;

  for (int i = 0; i < threadc; i++)
  {
    if (pthread_join(threads[i], &returnval))
    {
      fatal(i);
    }
  }

  /* TEST QUEUE */
  struct queue q;
  queue_init(&q);

  for (long i = 0; i < 10; i++) {
    printf("INSERTING %ld\n", i);
    queue_insert(&q, i);
  }

  printf("queue len: %d\n", q.size);

  for (long i = 0; i < 10; i++) {
    long l = queue_pop(&q);
    printf("POPPED %ld\n", l);
  }
}