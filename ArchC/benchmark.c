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
#include <limits.h>

#include <math.h>

int process_int(const char **, char *, va_list * );
int my_scanf4(const char *, ...);
int my_scanf5(const char *, ...);
ssize_t low_read(int, const void *, size_t);

/*#include <sysdep.h>*/

/******************************************************************************/
/*
#ifdef COMPILE_WSCANF
int
  __vfwscanf_internal (FILE *s, const char *format, va_list argptr, unisgned int mode_flags)
#else 
int 
  __vscanf_internal(FILE *s, const char *format, va_list argptr, unsigned int mode_flags)
#endif
*/



//Definizioni necessarie
#define BUFFER_SIZE 1024
#define SYS_write 1
#define SYS_read 0

#define MAX_SAFE (INT_MAX/10)
#define INT_MAX_LAST_DIGIT (INT_MAX % 10)


int main(void) {


  return 0;
}


int process_int(const char **format, char *input, va_list *args) {
  int *num = va_arg(*args, int *);
  int64_t value = 0;
  int sign = 1;

  //skip spaces
  while (isspace(*input)){ input++;}

  //check sign
  if (*input == '-') {sign = -1; input++;}
  //const int MAX_SAFE=INT_MAX/10;

  //convert to integer
  while (*input && isdigit(*input)) {
    //check for overflow, the update value
    double Quotient = (double) INT_MAX / 10;
    Quotient = trunc(Quotient);
    
    if (value > MAX_SAFE ||(value == MAX_SAFE && (*input -'0') > INT_MAX_LAST_DIGIT)) 
      {perror("Scanf4: EINVAL: buffer overflow"); return -1;}
    value = value * 10 + (*input - '0');
    input++;
  }
  *num = value * sign;
  
  return 1; //SUCCESS!
}

int process_int1(const char **format, char *input, va_list *args) {
  int *num = va_arg(*args, int *);
  int64_t value = 0;
  int sign = 1;

  //skip spaces
  while (isspace(*input)){ input++;}

  //check sign
  if (*input == '-') {sign = -1; input++;}
  //const int MAX_SAFE=INT_MAX/10;

  //convert to integer
  while (*input && isdigit(*input)) {
    //check for overflow, the update value
    const double Quotient = trunc((double) INT_MAX / 10);
    const int INT_MAX_LAST_DIGIT_QUOTIENT = INT_MAX%10;
    const int MAXIMUM_SAFE= (int)Quotient;

    if (value > MAX_SAFE ||(value == MAXIMUM_SAFE && (*input -'0') > INT_MAX_LAST_DIGIT_QUOTIENT)) 
      {perror("Scanf4: EINVAL: buffer overflow"); return -1;}
    value = value * 10 + (*input - '0');
    input++;
  }
  *num = value * sign;
  
  return 1; //SUCCESS!
}


int my_scanf4(const char *format, ...) {
  va_list args;
  va_start(args, format);

  char buf[BUFFER_SIZE];
  ssize_t bytesRead = 0;
  char *input = buf;

  //read input to buffer
  if ((bytesRead = low_read(0, buf, BUFFER_SIZE -1)) <=0) {
      va_end(args);
      return -1; //Error or EOF
  }
  buf[bytesRead] = '\0'; //NULL termination
  int numAssigned =0;

  //Parse *format
  while (*format) {
    if (*format == '%') {
      format++;
      switch (*format) {
        case 'd':
          numAssigned +=process_int(&format, input, &args);
          break;
        default:
          va_end(args);
          perror("SCANF4: EBADF");
          return -1;

      }
    } else {format++;}
  }
  va_end(args);
  return numAssigned;
}

ssize_t low_read(int fd, const void *buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		puts("EBADF");
		return -1;
	}
	if (buf==NULL) {
		errno = EINVAL;
		puts("EINVAL");
		return -1;
	}
	ssize_t result = syscall(SYS_read, fd, buf, count);
	if (result < 0) {
		printf("Syscall_read failed, return%ld\n", result);
	}
}

void benchmark(int (*process_func)(const char **, char *, va_list *)) {
    char input[100];
    for (int i = 0; i < 1e6; i++) {
        sprintf(input, "%d", rand() % INT_MAX);
        va_list args;
        va_start(args, input);
        process_func("%d", input, &args);
        va_end(args);
    }
}

