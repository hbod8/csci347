/* CS 347 -- Micro Shell!  
 *
 *   Sept 21, 2000,  Phil Nelson
 *   Modified April 8, 2001 
 *   Modified January 6, 2003
 *   Modified January 8, 2017
 * 
 *   Modified for assignment 1 by Harry Saliba
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "globals.h"
#include "defn.h"

/* Constants */

#define LINELEN 1024

/* Shell main */

int mainargc;
char **mainargv;
int shift;
FILE *infile;

int main(int argc, char **argv)
{
  char buffer[LINELEN];
  int len;

  mainargv = argv;
  mainargc = argc;

  infile = stdin;

  if (argc > 1)
  {
    infile = fopen(mainargv[1], "r");
    if (infile == 0)
    {
      perror("Error opening file.");
      exit(127);
    }
  }

  while (1)
  {

    /* prompt and get line */
    if (infile == stdin)
    {
      fprintf(stderr, "%% ");
    }
    if (fgets(buffer, LINELEN, infile) != buffer)
      break;

    /* Get rid of \n at end of buffer. */
    len = strlen(buffer);
    if (buffer[len - 1] == '\n')
      buffer[len - 1] = 0;

    /* Remove comments */
    removeComments(buffer);

    /* Run it ... */
    processline(buffer);
  }

  if (!feof(infile))
    perror("read");

  return 0; /* Also known as exit (0); */
}

void processline(char *line)
{
  pid_t cpid;
  int status;

  /* expand enviroment variables */
  char *processedLine = (char *)calloc(LINELEN, sizeof(char));
  if (expand(line, processedLine, LINELEN) < 0)
  {
    return;
  }

  /* process the args */
  int argc = 0;
  char **argv = arg_parse(processedLine, &argc);
  if (argc == 0)
  {
    return;
  }

  /* check for built-ins */
  if (shellcommand(argv, argc) == 0)
  {
    return;
  }

  /* Start a new process to do the job. */
  cpid = fork();
  if (cpid < 0)
  {
    /* Fork wasn't successful */
    perror("fork");
    return;
  }

  /* Check for who we are! */
  if (cpid == 0)
  {
    /* We are the child! */
    execvp(argv[0], argv);
    /* execlp reurned, wasn't successful */
    perror("exec");
    fclose(infile); // avoid a linux stdio bug
    exit(127);
  }

  /* Have the parent wait for child to complete */
  if (wait(&status) < 0)
  {
    /* Wait wasn't successful */
    perror("wait");
  }
}

/*Parse Arguments
 * Processes a string containing an executable command into an array of arguments.
 * 
 * @TODO: process command line arguments that have double qoutes.
 */
char **arg_parse(char *line, int *argcptr)
{
  /* count argv */
  int count = 0;
  char *pos = line;
  while (*pos != '\0')
  {
    if (*pos == ' ')
    {
      pos++;
    }
    else
    {
      count++;
      while (*pos != '\0' && *pos != ' ')
      {
        if (*pos == '"')
        {
          pos++;
          while (*pos != '"')
          {
            if (*pos == '\0')
            {
              printf("Wrong number of double qoutes.\n");
              return NULL;
            }
            pos++;
          }
          pos++;
        }
        else
        {
          pos++;
        }
      }
    }
  }
  *argcptr = count;

  /* malloc size + 1 */
  char **argv = (char **)malloc(sizeof(char *) * (count + 1));
  argv[count] = NULL;

  /* assign pointers and add EOS chars */
  count = 0;
  pos = line;
  while (*pos != '\0')
  {
    if (*pos == ' ')
    {
      pos++;
    }
    else
    {
      argv[count] = pos;
      count++;
      while (*pos != '\0' && *pos != ' ')
      {
        if (*pos == '"')
        {
          pos++;
          while (*pos != '"')
          {
            pos++;
          }
          pos++;
        }
        else
        {
          pos++;
        }
      }
      if (*pos != '\0')
      {
        *pos = '\0';
        pos++;
      }
    }
  }

  /*remove qoutes */
  count--;
  char *nq;
  while (count >= 0)
  {
    pos = argv[count];
    nq = argv[count];
    while (*nq != '\0' && *pos != '\0')
    {
      while (*nq == '"')
      {
        nq++;
      }
      *pos = *nq;
      pos++;
      nq++;
    }
    *pos = '\0';
    count--;
  }

  return argv;
}

/* remove comments */
int removeComments(char *line)
{
  char *ptr = line;
  while (*ptr != '\0')
  {
    if (*ptr == '$')
    {
      ptr++;
      if (*ptr == '#')
      {
        ptr++;
      }
    }
    else if (*ptr == '#')
    {
      *ptr = '\0';
    }
    else
    {
      ptr++;
    }
  }
  return 0;
}