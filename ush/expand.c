#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <string.h>
#include "globals.h"

int expand(char *orig, char *new, int newsize)
{
  /* make sure the last charachter of the output string is \0 */
  new[newsize] = '\0';

  /* process */
  int dst = 0;
  int src = 0;
  // Loop until EOS or overflow
  while (orig[src] != '\0' && dst < newsize)
  {
    // if start of expand
    if (orig[src] == '$')
    {
      src++;
      // if env var
      if (orig[src] == '{')
      {
        src++;
        // remember the start of the name
        char *varName = &orig[src];
        // loop until ending brace
        while (orig[src] != '}')
        {
          // be sure not EOS, if EOS then error no ending brace
          if (orig[src] == '\0')
          {
            //err
            fprintf(stderr, "a missing } error\n");
            return -1;
          }
          src++;
        }
        orig[src] = '\0';
        src++;
        // Attempt to get variable
        char *var = getenv(varName);
        if (var)
        {
          // Copy in variable
          while (*var != '\0' && dst < newsize)
          {
            new[dst] = *var;
            dst++;
            var++;
          }
        }
      }
      // Expand PID
      else if (orig[src] == '$')
      {
        src++;
        int size = snprintf(&new[dst], newsize - dst, "%d", getpid());
        dst += size;
      }
      // expand argument
      else if (isdigit(orig[src]))
      {
        char *val = &orig[src];
        while (isdigit(orig[src]))
        {
          src++;
        }
        char save = orig[src];
        orig[src] = '\0';
        int argnum = atoi(val) + 1;
        orig[src] = save;
        if (mainargv[1] != NULL)
        {
          argnum = 0;
        }
        if (argnum < mainargc - shift)
        {
          int size = snprintf(&new[dst], newsize - dst, "%s", mainargv[argnum + shift]);
          dst += size;
        }
      }
      else if (orig[src] == '#')
      {
        int scriptrun = 0;
        if (mainargv[1] != NULL)
        {
          scriptrun = 1;
        }
        src++;
        int size = snprintf(&new[dst], newsize - dst, "%d", mainargc - scriptrun - shift);
        dst += size;
      }
      // Add any other expansion rules here
    }
    // if start of wildcard
    else if (orig[src] == '*' && (orig[src - 1] == ' ' || orig[src - 1] == '\0') && (orig[src + 1] == ' ' || orig[src + 1] == '\0'))
    {
      src++;
      struct dirent* file;
      DIR *d = opendir(".");
      while ((file = readdir(d)))
      {
        char *fname = file->d_name;
        int size = snprintf(&new[dst], newsize - dst, "%s ", fname);
        dst += size;
      }
      dst--;
    }
    else if (orig[src] == '*' && (orig[src - 1] == ' ' || orig[src - 1] == '\0'))
    {
      src++;
      char *match = &orig[src];
      while (orig[src] != ' ' && orig[src] != '\0')
      {
        src++;
      }
      char save = orig[src];
      orig[src] = '\0';
      struct dirent* file;
      DIR *d = opendir(".");
      while ((file = readdir(d)))
      {
        char *fname = file->d_name;
        if ((strlen(match) < strlen(fname)) && (strcmp(fname + strlen(fname) - strlen(match), match) == 0))
        {
          int size = snprintf(&new[dst], newsize - dst, "%s ", fname);
          dst += size;
        }
      }
      dst--;
      orig[src] = save;
    }
    else
    {
      new[dst] = orig[src];
      src++;
      dst++;
    }
  }

  if (dst >= newsize)
  {
    fprintf(stderr, "an expansion overflow error\n");
    return -1;
  }
  // puts(new);
  return 0;
}