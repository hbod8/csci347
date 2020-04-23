#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

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
            return -1;
          }
          src++;
        }
        orig[src] = '\0';
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
      if (orig[src] == '$')
      {
        src++;
        int size = snprintf(&new[dst], newsize - dst, "%d", getpid());
        dst += size;
      }
      // Add any other expansion rules here
    }
    // copy
    new[dst] = orig[src];
    src++;
    dst++;
  }
  // puts(new);
  return 0;
}