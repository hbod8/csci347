/* Simple matrix multiply program with pthread implementation
 *
 * Phil Nelson, March 5, 2019
 * Modified by Harry Saliba June 9, 2020
 *
 */

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

/* idx macro calculates the correct 2-d based 1-d index
 * of a location (x,y) in an array that has col columns.
 * This is calculated in row major order. 
 */
#define idx(x, y, col) ((x) * (col) + (y))

struct partialmatmul {
  double *A;
  double *B;
  double *C;
  int x;
  int y;
  int z;
  int i;
  int n;
  int threadn;
  int square;
};

/* Fatal error handler */
void fatal(int n)
{
  fprintf(stderr, "Fatal error on thread %d.\n", n);
  exit(n);
}

void partialMatMul(struct partialmatmul *this) {
  int ix, jx, kx;

  /* rows of the solution */
  for (ix = this->i; ix < this->i + this->n; ix++)
  {
    /* columns of the solution */
    double tval = 0;
    for (kx = 0; kx < this->x; kx++)
    {
      // Sum the A row time B column
      tval += this->A[idx(ix / this->y, kx, this->y)] * this->B[idx(kx, ix % this->z, this->z)];
      /* Debug */
      // printf("Thread %d: C[%d, %d] += A[%d, %d] * B[%d, %d]\n", this->threadn, ix / this->y, ix % this->z, ix / this->y, kx, kx, ix % this->z);
    }
    this->C[ix] = tval;
  }
}

void *thread_body(void *arg) {
  struct partialmatmul *this = (struct partialmatmul *)arg;
  if (this->square) {
    int i;

    partialMatMul(this);
    if (this->square > 1)
    {
      // Need a Temporary for the computation
      double *T = (double *)malloc(sizeof(double) * this->x * this->x);
      for (i = 1; i < this->square; i += 2)
      {
        this->A = this->B;
        this->B = this->B;
        this->C = T;
        partialMatMul(this);
        if (i == this->square - 1)
        {
          memcpy(this->B, T, sizeof(double) * this->x * this->x);
        }
        else
        {
          this->A = T;
          this->B = T;
          this->C = this->B;
          partialMatMul(this);
        }
      }
      free(T);
    }
  } else {
    partialMatMul(this);
  }
}

/* Matrix Multiply:
 *  C (x by z)  =  A ( x by y ) times B (y by z)
 *  This is the slow n^3 algorithm
 *  A and B are not be modified
 */
void MatMul(double *A, double *B, double *C, int x, int y, int z, int nThreads)
{
  /* Create threads */
  int size = x * z;
  int index = 0;
  struct partialmatmul tm[nThreads];
  pthread_t threads[nThreads];
  for (int i = 0; i < nThreads; i++ ) {
    tm[i].A = A;
    tm[i].B = B;
    tm[i].C = C;
    tm[i].x = x;
    tm[i].y = y;
    tm[i].z = z;
    tm[i].i = index;
    int n = size / (nThreads - i);
    size -= n;
    index += n;
    tm[i].n = n;
    tm[i].threadn = i;
    tm[i].square = 0;
    if (pthread_create(&threads[i], NULL, thread_body, (void *)&tm[i]))
    {
      fatal(i);
    }
  }
  /* Join threads */
  void *returnval;
  for (int i = 0; i < nThreads; i++)
  {
    if (pthread_join(threads[i], &returnval))
    {
      fatal(i);
    }
  }
}

/* Matrix Square: 
 *  B = A ^ 2*times
 *
 *    A are not be modified.
 */
void MatSquare(double *A, double *B, int x, int times, int nThreads)
{
  /* Create threads */
  int size = x * x;
  int index = 0;
  struct partialmatmul tm[nThreads];
  pthread_t threads[nThreads];
  for (int i = 0; i < nThreads; i++ ) {
    tm[i].A = A;
    tm[i].B = A;
    tm[i].C = B;
    tm[i].x = x;
    tm[i].y = x;
    tm[i].z = x;
    tm[i].i = index;
    int n = size / (nThreads - i);
    size -= n;
    index += n;
    tm[i].n = n;
    tm[i].threadn = i;
    tm[i].square = times;
    if (pthread_create(&threads[i], NULL, thread_body, (void *)&tm[i]))
    {
      fatal(i);
    }
  }
  /* Join threads */
  void *returnval;
  for (int i = 0; i < nThreads; i++)
  {
    if (pthread_join(threads[i], &returnval))
    {
      fatal(i);
    }
  }
}

/* Print a matrix: */
void MatPrint(double *A, int x, int y)
{
  int ix, iy;

  for (ix = 0; ix < x; ix++)
  {
    printf("Row %d: ", ix);
    for (iy = 0; iy < y; iy++)
      printf(" %10.5G", A[idx(ix, iy, y)]);
    printf("\n");
  }
}

/* Generate data for a matrix: */
void MatGen(double *A, int x, int y, int rand)
{
  int ix, iy;

  for (ix = 0; ix < x; ix++)
  {
    for (iy = 0; iy < y; iy++)
    {
      A[idx(ix, iy, y)] = (rand ? ((double)(random() % 200000000)) / 2000000000.0 : (1.0 + (((double)ix) / 100.0) + (((double)iy / 1000.0))));
    }
  }
}

/* Print a help message on how to run the program */
void usage(char *prog)
{
  fprintf(stderr, "%s: [-dr] -x val -y val -z val\n", prog);
  fprintf(stderr, "%s: [-dr] -s num -x val\n", prog);
  exit(1);
}

/* Main function
 *
 *  args:  -d   -- debug and print results
 *         -r   -- use random data between 0 and 1 
 *         -s t -- square the matrix t times 
 *         -x   -- rows of the first matrix, r & c for squaring
 *         -y   -- cols of A, rows of B
 *         -z   -- cols of B
 *         
 */
int main(int argc, char **argv)
{
  extern char *optarg; /* defined by getopt(3) */
  int ch;              /* for use with getopt(3) */

  /* option data */
  int x = 0, y = 0, z = 0;
  int debug = 0;
  int square = 0;
  int useRand = 0;
  int sTimes = 0;
  int timeExec = 0;
  int numThreads = 8;

  while ((ch = getopt(argc, argv, "Tdrs:x:y:z:n:")) != -1)
  {
    switch (ch)
    {
    case 'd': /* debug */
      debug = 1;
      break;
    case 'r': /* debug */
      useRand = 1;
      srandom(time(NULL));
      break;
    case 's': /* s times */
      sTimes = atoi(optarg);
      square = 1;
      break;
    case 'x': /* x size */
      x = atoi(optarg);
      break;
    case 'y': /* y size */
      y = atoi(optarg);
      break;
    case 'z': /* z size */
      z = atoi(optarg);
      break;
    case 'n': /* z size */
      numThreads = atoi(optarg);
      break;
    case 'T': /* time option */
      timeExec = 1;
      break;
    case '?': /* help */
    default:
      usage(argv[0]);
    }
  }

  /* verify options are correct. */
  if (square)
  {
    if (y != 0 || z != 0 || x <= 0 || sTimes < 1)
    {
      fprintf(stderr, "Inconsistent options\n");
      usage(argv[0]);
    }
  }
  else if (x <= 0 || y <= 0 || z <= 0)
  {
    fprintf(stderr, "-x, -y, and -z all need"
                    " to be specified or -s and -x.\n");
    usage(argv[0]);
  }

  /* Matrix storage */
  double *A;
  double *B;
  double *C;

  if (square)
  {
    A = (double *)malloc(sizeof(double) * x * x);
    B = (double *)malloc(sizeof(double) * x * x);
    MatGen(A, x, x, useRand);
    if (timeExec)
    {
      clock_t start = clock();
      MatSquare(A, B, x, sTimes, numThreads);
      clock_t end = clock();
      printf("CPU time elapsed: %f\n", ((double)(end - start) / CLOCKS_PER_SEC));
    }
    else
    {
      MatSquare(A, B, x, sTimes, numThreads);
    }
    if (debug)
    {
      printf("-------------- orignal matrix ------------------\n");
      MatPrint(A, x, x);
      printf("--------------  result matrix ------------------\n");
      MatPrint(B, x, x);
    }
  }
  else
  {
    A = (double *)malloc(sizeof(double) * x * y);
    B = (double *)malloc(sizeof(double) * y * z);
    C = (double *)malloc(sizeof(double) * x * z);
    MatGen(A, x, y, useRand);
    MatGen(B, y, z, useRand);
    if (timeExec)
    {
      clock_t start = clock();
      MatMul(A, B, C, x, y, z, numThreads);
      clock_t end = clock();
      printf("CPU time elapsed: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
    }
    else
    {
      MatMul(A, B, C, x, y, z, numThreads);
    }
    if (debug)
    {
      printf("-------------- orignal A matrix ------------------\n");
      MatPrint(A, x, y);
      printf("-------------- orignal B matrix ------------------\n");
      MatPrint(B, y, z);
      printf("--------------  result C matrix ------------------\n");
      MatPrint(C, x, z);
    }
  }
  return 0;
}
