#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#define KAYAK 1
#define CANOE 2
#define SAILBOAT 4

#define LINE_TOO_LONG 1

struct group
{
  int number;
  int lifejackets;
};

/* Command line attributes */
/* Number of threads. */
int threadc;
/* Rate at which groups arrive to Lakewood */
int grouprate;

/* Fatal error handler */
void fatal(int n)
{
  fprintf(stderr, "Fatal error on thread %d.\n", n);
  exit(n);
}

/* Fatal error handler with message */
void fatalm(int n, char *msg, int errnum)
{
  char *errmsg = "";
  switch (errnum) {
    case EINVAL:
    errmsg = "EINVAL";
    break;
    case EBUSY:
    errmsg = "EBUSY";
    break;
    case EDEADLK:
    errmsg = "EDEADLK";
    break;
    case EPERM:
    errmsg = "EPERM";
  }
  fprintf(stderr, "Fatal error on thread %d. %s:%s\n", n, msg, errmsg);
  exit(n);
}

/* Queue */

/* Queue node */
struct node
{
  int data;
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
void queue_push(struct queue *queue, int value)
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
int queue_pop(struct queue *queue)
{
  int retval = 0;
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
int getlifejackets(int n, int threadn, int *remaining)
{
  /* lock mutex */
  int err = pthread_mutex_lock(&ljmutex);
  if (err)
  {
    fatalm(threadn, "Failed to lock when getting life jackets", err);
  }
  /* make sure there is space in the queue */
  if (groupqueue.size >= 5)
  {
    /* line was too long */
    printf("The line was too long for group %d, they left.\n", threadn);
    /* unlock mutex */
    err = pthread_mutex_unlock(&ljmutex);
    if (err)
    {
      fatalm(threadn, "Failed to unlock when getting life jackets (line too long)", err);
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
      err = pthread_cond_wait(&donecond, &ljmutex);
      if (err) {
        fatalm(threadn, "Failed to wait when getting life jackets", err);
      }
    }
    /* leave line */
    queue_pop(&groupqueue);
  }
  /* Take lifejackets */
  lifejackets -= n;
  *remaining = lifejackets;
  /* unlock mutex */
  err = pthread_mutex_unlock(&ljmutex);
  if (err)
  {
    fatalm(threadn, "Failed to unlock when getting life jackets", err);
  }
  return 0;
}

/* Return lifejackets */
int returnlifejackets(int n, int threadn, int *remaining)
{
  /* lock mutex */
  int err = pthread_mutex_lock(&ljmutex);
  if (err)
  {
    fatalm(threadn, "Failed to lock when returning lifejackets", err);
  }
  /* return lifejackets */
  lifejackets += n;
  *remaining = lifejackets;
  /* unlock mutex */
  err = pthread_mutex_unlock(&ljmutex);
  if (err)
  {
    fatalm(threadn, "Failed to unlock when returning lifejackets", err);
  }
  /* broadcast done */
  err = pthread_cond_broadcast(&donecond);
  if (err)
  {
    fatalm(threadn, "Failed to broadcast when returning lifejackets", err);
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
  printf("Group %d wants to rent a %s and needs %d lifejackets.\n", g->number, gettype(g->lifejackets), g->lifejackets);
}

/* Print thread info */
void printgroupusing(struct group *g, int remaining)
{
  printf("Group %d is using a %s and %d lifejackets. There are %d lifejackets left.\n", g->number, gettype(g->lifejackets), g->lifejackets, remaining);
}

/* Print thread info */
void printgroupdone(struct group *g, int remaining)
{
  printf("Group %d is done using a %s and %d lifejackets. There are %d lifejackets left.\n", g->number, gettype(g->lifejackets), g->lifejackets, remaining);
}

/* Thread Entry Point */
void *thread_body(void *arg)
{
  struct group *this = (struct group *)arg;
  /* print info */
  printgroup(this);
  /* get lifejackets */
  int remainingljs = 0;
  int err = getlifejackets(this->lifejackets, this->number, &remainingljs);
  if (err == LINE_TOO_LONG) {
    pthread_exit(arg);
  } else if (err) {
    fatal(this->number);
  }
  /* use life jackets */
  printgroupusing(this, remainingljs);
  sleep(rand() % 8);
  /* return lifejackets */
  returnlifejackets(this->lifejackets, this->number, &remainingljs);
  printgroupdone(this, remainingljs);
  pthread_exit((void *)arg);
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