/* An example of global variables across several (2) files */

#pragma once

/* C preprocessor defines for definition and initialization */

#ifdef DEFINE_GLOBALS

#define GLOBAL_VAR(type, name, init) \
  extern type name;                  \
  type name = init

#else

#define GLOBAL_VAR(type, name, init) extern type name

#endif

/* Actual global variables */

GLOBAL_VAR(int, mainargc, 0);

GLOBAL_VAR(char **, mainargv, NULL);

GLOBAL_VAR(int, shift, 0);