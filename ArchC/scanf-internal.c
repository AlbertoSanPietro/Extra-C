#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include <termios.h>
#include <features.h>
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <stdbool.h>
//#include "libioP.h"

#ifdef COMPILE_WSCANF
int
  __vfwscanf_internal (FILE *s, const char *format, va_list argptr, unisgned int mode_flags)
#else 
int 
  __vfscanf_internal(FILE *s, const char *format, va_list argptr, unsigned int mode_flags)
#endif

int my_scanf(const char *format, ...) {
  va_list arg;
  int done;

  va_start(arg, format);
  done = __vfscanf_internal(stdin, format, arg, 0);
  va_end (arg);
  
  return done;
}





































