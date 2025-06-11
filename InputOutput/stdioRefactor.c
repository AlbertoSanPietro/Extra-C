/*"minimal" rewrite of stdio.h to better understand some inner workings of it.
 *The code here is based on the glibc, but it is very much simplified and less robust
 */

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
#include <wctype.h>
#include <wchar.h>
#include <stdbool.h>
#include <limits.h>

//Some necessary defines
#define BUFFER_SIZE 1024
#define SYS_write 1
#define SYS_read 0
#define INT_MAX_LAST_DIGIT (INT_MAX % 10)

#define ungetc(c, s)	((void) (c == WEOF				      \
				 || (--read_in,				      \
				     _IO_sputbackwc (s, c))))

//Global Variables
char buffer[BUFFER_SIZE];
size_t buffer_index = 0;
char getch_buf[BUFFER_SIZE];
int bufp = 0;
char gchar;

/**************************************************************/
//OUTPUT functions (stdout)
ssize_t my_write(int, const void *, size_t);
ssize_t low_write(int, const void *, size_t);
void print_string(const char *);
void print_float(double, int);
void print_int(int);
void my_printf(const char *, ...);
int my_puts(const char *);
int my_putchar(int);
void my_native_putchar(char);
void print_int64_t(int64_t);

//Getch and Ungetch...
int getch(void);
void ungetch(int);
/*******************************************************************************************/
//INPUT functions (stdin)
int my_getchar(void);
ssize_t bytes;
ssize_t low_read(int, const void *, size_t);
int my_scanf4(const char *, ...);

/************************************************/
//Helper functions
int power10(int);
int64_t int64_power10(int64_t);
int string_to_int(const char *);

/****************************************************************/
int my_getchar(void) {
	ssize_t readByte;
	if ((readByte = low_read(STDIN_FILENO, &gchar, 1))== 1) {
		return (unsigned char)gchar;
	}
	else return EOF;
}

ssize_t my_write(int fd, const void *buf, size_t count) {
	if (fd== STDOUT_FILENO || fd == 1){
		if (buffer_index + count > BUFFER_SIZE) {
			puts("Buffer overflow, get fucked bozo");
			buffer_index = 0;
		}

		memcpy(buffer+buffer_index, buf, count);
		buffer_index +=count;
		low_write(STDOUT_FILENO, buf, count);
		return count;
	}
	return -1;
}

int my_puts(const char *str) {
	int r=0;
	for(const char *c = str; *c!=0; c++) {
		my_native_putchar((int)*c);
		r++;
	}
	if(r) {
		my_native_putchar('\n');
		r++;
	}
	return r ? r: EOF;
}

int my_putchar(int c) {
	if (low_write(1, &c, 1) != 1) {
		return EOF;
	}
	return c;
}

void my_native_putchar(char character) {
	low_write(1, &character, 1);
}

ssize_t low_write(int fd, const void *buf, size_t count) {
	if (fd<0) {
		errno = EBADF;
		puts("EBADF");
		return -1;
	}
	if (buf == NULL) {
		errno=EINVAL;
		puts("EINVAL");
		return -1;
	}

	ssize_t result=syscall(SYS_write, fd, buf, count);

	if (result <0) {
		printf("Syscall failed with error code:%ld\n", result);
		return -1;
	}
	return result;
}

ssize_t low_read(int fd, const void *buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		my_puts("EBADF");
		return -1;
	}
	if (buf==NULL) {
		errno = EINVAL;
		my_puts("EINVAL");
		return -1;
	}
	ssize_t result = syscall(SYS_read, fd, buf, count);
	if (result < 0) {
		my_printf("Syscall_read failed, return%ld\n", result);
	}
	return result;
}

void print_string(const char *str) {
	while (*str) {
		low_write(STDOUT_FILENO, str, 1);
		str++;
	}
}

void print_float(double num, int precision) {
	if (num<0) {
		low_write(1, "-", 1);
		num=-num;
	}
	int int_part = (int)num;
	print_int(int_part);
	low_write(STDOUT_FILENO, ".", 1);
	
	double fraction = num -int_part;
	int fraction_as_int = (int)(fraction * power10(precision));
	print_int(fraction_as_int);
}

void print_double(double num, int64_t precision) {
	if (num < 0) {
		low_write(1, "-", 1);
		num = -num;
	}
	
	int64_t int_part = (int64_t)num;
	print_int64_t(int_part);
	low_write(1, ".", 1);
	
	double fraction = num - int_part;
	
	int64_t fraction_as_int = (int64_t)(fraction * int64_power10(precision));
	if (fraction_as_int < 0) fraction_as_int = -fraction_as_int;
	print_int64_t(fraction_as_int);
}

int64_t int64_power10(int64_t n) {
	int64_t result = 1;
	while(n--) {
		result *=10;
	}
	return result;
}

int power10(int n) {
	int result = 1;
	while(n--) {
		result *=10;
	}
	return result;
}

int getch(void) {
	return (bufp > 0) ? getch_buf[--bufp] : my_getchar();
}

void ungetch(int c) {
	if (bufp>=BUFFER_SIZE)
		my_printf("Ungetch: buffer overflow\n");
	else 
		getch_buf[bufp++] = c;
}

void print_int(int n) {
	char buf[20];
	int i =0;

	if (n<0) {
		low_write(STDOUT_FILENO, "-", 1);
		n=-n;
	}
	do {
		buf[i++] = (n%10) + '0';
		n/=10;
	} while (n > 0);

	while (i--) {
		low_write(STDOUT_FILENO, &buf[i], 1);
	}
}

void print_int64_t(int64_t n) {
	char buf[129];
	int64_t i=0;
	if (n<0) {
		low_write(STDOUT_FILENO, "-",1);
		n=-n;
	}
	do {
		buf[i++] = (n%10) + '0';
		n/=10;
	} while (n > 0);
	while (i--) {
		low_write(1, &buf[i], 1);
	}
}

void my_printf(const char *format, ...) {
	va_list args;
	va_start(args, format);

	for (const char *p = format; *p; p++) {
		if (*p=='%') {
			p++;
			switch (*p) {
				case 'd': {
					  int i = va_arg(args, int);
					  print_int(i);
					  break;
					  }
				case 's': {
					  char *s = va_arg(args, char *);
					  print_string(s);
					  break;
					  }
				case 'f': {
					  double i = va_arg(args, double);
					  print_float(i, 6);
					  break;
					  }
				case 'l': {
					p++;
					switch (*p++) {
						case 'f':
							{
							double k = va_arg(args, double);
							print_double(k, 12);
							break;
							}
						case 'd':
							int64_t k = va_arg(args, int64_t);
							print_int64_t(k);
							break;

						default :
							low_write(STDOUT_FILENO,p, 1);
						}
					}
				
				default:
					  low_write(STDOUT_FILENO, p, 1);
			}
		} else if (*p == '\\') {
			switch (*p) {
				case 'n': {
					 low_write(STDOUT_FILENO, "\n", 1);
					  break;
					  }
				case 't': {
					  low_write(STDOUT_FILENO, "\t", 1);
					  break;
					  }
				case '\'': {
					  low_write(STDOUT_FILENO, "\'", 1);
					  break;
					  }

				default:
					  low_write(STDOUT_FILENO, "\\", 1);
					  low_write(STDOUT_FILENO, p, 1);
			
			}
		} 
		else {low_write(STDOUT_FILENO, p, 1);}
	}

va_end(args);
}

int string_to_int(const char *str) {
	int num=0;
	while(*str) {
		if (isdigit(*str)) {
			num = num * 10 + (*str - '0');
		} else 
			break;
	}
	return num;
}

int process_int(const char **format, char **og_input, va_list *args) {
	int64_t *num = va_arg(*args, int64_t *);
	char *input = *og_input;
	int64_t value = 0;
	int sign = 1;

	while (isspace(*input)){ input++;}

	if (*input == '-') {sign = -1; input++;}
	const int MAX_SAFE=INT_MAX/10;

	while (*input && isdigit(*input)) {
		if (value > MAX_SAFE ||(value == MAX_SAFE && (*input -'0') > (INT_MAX) % 10)) 
			{perror("Scanf4: EINVAL: buffer overflow"); return 1;}
		value = value * 10 + (*input - '0');
		input++;
	}
	*num = value * sign;
	
	return 0;
}

int64_t process_int64_t(const char **format, char **og_input, va_list *args) {
	int64_t *num = va_arg(*args, int64_t *);
	char *input= *og_input;
	int64_t value=0;
	const int64_t MAX_SAFE=INT64_MAX/10;
	int sign = 1;
	
	while(isspace(*input)) {input++;}
	if (*input == '-') {sign = -1; input++;}

	while (*input && isdigit(*input)) {
		if (value > MAX_SAFE ||(value == MAX_SAFE && (*input -'0') > (INT_MAX) % 10)) {
			perror("int64_t: Buffer overflow");
			return 1;
		}
		value = value * 10 + (*input -'0');
		input++;
	}

	*num = value * sign;
	return 1;
}

int my_scanf4(const char *format, ...) {
	va_list args;
	va_start(args, format);

	char buf[BUFFER_SIZE];
	ssize_t bytesRead = 0;
	char *input = buf;

	if ((bytesRead = low_read(0, buf, BUFFER_SIZE -1)) <=0) {
		va_end(args);
		return -1;
	}
	buf[bytesRead] = '\0';
	int64_t numAssigned =0;
	
	while(*format) {
		if (*format == '%') {
			format++;
			switch (*format) {
				case 'd':
					puts("int");
					numAssigned += process_int(&format, &input, &args);
					break;
				case 'l':
					format++;
					switch(*format++) {
						case 'd':
							numAssigned += process_int64_t(&format, &input, &args);
							break;
					}
					break;
				default:
					va_end(args);
					perror("SCANF4: EBADF");
					return 1;
			}
		}
	}
	va_end(args);
	return numAssigned;
}

int main(void) {
	int64_t scanf4; 
	my_scanf4("%ld", &scanf4);
	my_printf("scanf4: %ld\n", scanf4);
	return 0;
}

