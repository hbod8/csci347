#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define KAYAK 1
#define CANOE 2
#define SAILBOAT 4

#define LINE_TOO_LONG 1
#define NOT_ENOUGH_LIFEJACKETS 2

struct group
{
  long number;
  int lifejackets;
};

/* Command line attributes */
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
struct node
{
  long data;
  struct node *next;
};

/* Queue structure */
struct queue
{
  struct node *head;
  struct node *tail;
  int size;
};

/* Shared queue instance */
struct queue groupqueue;

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

/* Queue push */
void queue_push(struct queue *queue, long value)
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

/* Number of available life jackets. */
int lifejackets = 10;

/* Lifejacket mutex */
pthread_mutex_t ljmutex = PTHREAD_MUTEX_INITIALIZER;

/* Group done condition variable. */
pthread_cond_t donecond = PTHREAD_COND_INITIALIZER;

/* Get lifejackets */
int getlifejackets(int n, long threadn, int *remaining)
{
  /* lock mutex */
  if (pthread_mutex_trylock(&ljmutex))
  {
    fatal(threadn);
  }
  /* make sure there is space in the queue */
  if (groupqueue.size > 5)
  {
    /* line was too long */
    printf("The line was too long for group %ld, they left.\n", threadn);
    /* unlock mutex */
    if (pthread_mutex_unlock(&ljmutex))
    {
      fatal(threadn);
    }
    return LINE_TOO_LONG;
  }
  /* is there a line to join? */
  if (!queue_isEmpty(&groupqueue) || lifejackets < n)
  {
    /* get in line */
    queue_push(&groupqueue, threadn);
    /* wait for turn and lifejackets */
    while (groupqueue.head->data != threadn || lifejackets < n)
    {
      pthread_cond_wait(&donecond, &ljmutex);
    }
    /* leave line */
    queue_pop(&groupqueue);
  }
  /* Take lifejackets */
  lifejackets -= n;
  *remaining = lifejackets;
  /* unlock mutex */
  if (pthread_mutex_unlock(&ljmutex))
  {
    fatal(threadn);
  }
  return 0;
}

/* Return lifejackets */
int returnlifejackets(int n, long threadn, int *remaining)
{
  /* lock mutex */
  if (pthread_mutex_trylock(&ljmutex))
  {
    fatal(threadn);
  }
  /* return lifejackets */
  lifejackets += n;
  *remaining = lifejackets;
  /* unlock mutex */
  if (pthread_mutex_unlock(&ljmutex))
  {
    fatal(threadn);
  }
  /* broadcast done */
  if (pthread_cond_broadcast(&donecond))
  {
    fatal(threadn);
  }
  return 0;
}

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

/* Print thread info */
void printgroupusing(struct group *g, int remaining)
{
  printf("Group %ld is using a %s and %d lifejackets. There are %d lifejackets left.\n", g->number, gettype(g->lifejackets), g->lifejackets, remaining);
}

/* Print thread info */
void printgroupdone(struct group *g, int remaining)
{
  printf("Group %ld is done using a %s and %d lifejackets. There are %d lifejackets left.\n", g->number, gettype(g->lifejackets), g->lifejackets, remaining);
}

/* Thread Entry Point */
void *thread_body(void *arg)
{
  struct group *this = (struct group *)arg;
  /* print info */
  printgroup(this);
  /* get lifejackets */
  int remainingljs = 0;
  if (getlifejackets(this->lifejackets, this->number, &remainingljs)) {
    pthread_exit(arg);
  }
  /* use life jackets */
  printgroupusing(this, remainingljs);
  sleep(rand() % 8);
  /* return lifejackets */
  returnlifejackets(this->lifejackets, this->number, &remainingljs);
  printgroupdone(this, remainingljs);
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

  /* Initialize queue */
  queue_init(&groupqueue);

  /* Create threads */
  pthread_t threads[threadc];

  for (int i = 0; i < threadc; i++)
  {
    if (pthread_create(&threads[i], NULL, thread_body, (void *)&groups[i]))
    {
      fatal(i);
    }
    sleep(rand() % grouprate);
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
}