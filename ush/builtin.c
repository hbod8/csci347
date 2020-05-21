#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
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
      shift -= numshift;
      return 0;
    }
    else if (argc == 1)
    {
      shift = 0;
      return 0;
    }
    else
    {
      perror("Usage: unshift NUM");
      return 0;
    }
  }
  else if (strcmp(args[0], "sstat") == 0)
  {
    if (argc < 2) {
      perror("Usage: sstat file [file ...]");
      return 0;
    }
    struct stat *filestats;
    filestats = malloc(sizeof(struct stat));
    for (int i = 1; i < argc; i++) {
      if (stat(args[i], filestats) == 0) {
        struct passwd *u = getpwuid(filestats->st_uid);
        struct group *g = getgrgid(filestats->st_gid);
        char permissions[256];
        strmode(filestats->st_mode, &permissions);
        time_t modtime = filestats->st_mtime;
        printf("%s %s %s %s %ld %ld %s", args[i], u->pw_name, g->gr_name, permissions, filestats->st_nlink, filestats->st_size, asctime(localtime(&modtime)));
      }
      else
      {
        perror("Couldnt get filestats.");
        return 0;
      }
    }
    return 0;
  }
  return -1;
}