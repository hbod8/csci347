#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "globals.h"

int shellcommand(char **args, int argc)
{
  if (strcmp(args[0], "exit") == 0)
  {
    if (argc == 1)
    {
      exit(0);
    }
    else if (argc > 1)
    {
      exit(atoi(args[1]));
    }
    return 0;
  }
  else if (strcmp(args[0], "envset") == 0)
  {
    if (argc != 3)
    {
      perror("Usage: envset NAME value");
      return 0;
    }
    setenv(args[1], args[2], 1);
    return 0;
  }
  else if (strcmp(args[0], "envunset") == 0)
  {
    if (argc != 2)
    {
      perror("Usage: envunset NAME");
      return 0;
    }
    unsetenv(args[1]);
    return 0;
  }
  else if (strcmp(args[0], "cd") == 0)
  {
    if (argc == 2)
    {
      chdir(args[1]);
    }
    else if (argc == 1)
    {
      char *home = getenv("HOME");
      if (!home) {
        fprintf(stderr, "cd errors: HOME unset\n");
      }
      else
      {
        chdir(home);
      }
    }
    else
    {
      fprintf(stderr, "cd errors: Usage: cd [directory]\n");
      return 0;
    }
    return 0;
  }
  else if (strcmp(args[0], "shift") == 0)
  {
    if (argc != 2)
    {
      perror("Usage: shift NUM");
      return 0;
    }
    int numshift = atoi(args[1]);
    if (numshift >= mainargc)
    {
      perror("Too many shifts");
      return 0;
    }
    mainargc -= numshift;
    shift += numshift;
    return 0;
  }
  else if (strcmp(args[0], "unshift") == 0)
  {
    if (argc == 2)
    {
      int numshift = atoi(args[1]);
      if (numshift >= mainargc)
      {
        perror("Too many shifts");
        return 0;
      }
      mainargc += numshift;
      shift -= numshift;
      return 0;
    }
    else if (argc == 1)
    {
      mainargc += shift;
      shift = 0;
      return 0;
    }
    else
    {
      perror("Usage: unshift NUM");
      return 0;
    }
  }
  return -1;
}