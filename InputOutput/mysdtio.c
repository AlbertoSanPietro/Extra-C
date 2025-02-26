/*Riscrittura "minimale" di stdio.h perché sì*/

#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>

/*#include <sysdep.h>*/



//Definizioni necessarie
#define BUFFER_SIZE 1024
#define SYS_write 1
#define SYS_read 0
# define ungetc(c, s)	((void) (c == WEOF				      \
				 || (--read_in,				      \
				     _IO_sputbackwc (s, c))))


//Global Variables
char buffer[BUFFER_SIZE];
size_t buffer_index = 0;
char getch_buf[BUFFER_SIZE];
int bufp = 0;


/**************************************************************/
//Funzioni di OUTPUT (stdout)
ssize_t my_write(int, const void *, size_t);
ssize_t low_write(int, const void *, size_t);
void print_string(const char *);
void print_float(double, int);
void print_int(int);
void my_printf(const char *, ...);
int my_puts(const char *);
int my_putchar(int);
void my_native_putchar(char);
void print_long_int(long int);

//QUELLE funzioni
int getch(void);
void ungetch(int);
/*******************************************************************************************/
//Funzioni di INPUT (stdin)
int my_getchar(void);
char gchar;
ssize_t bytes;
ssize_t low_read(int, const void *, size_t);
int my_scanf1(const char *, ...);
int my_scanf2(const char *, ...);

/************************************************/
//Helper functions
int power10(int);
long int long_power10(long int);
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
			//memset(buffer,BUFFER_SIZE, 0);
		}

		memcpy(buffer+buffer_index, buf, count);
		buffer_index +=count;
		//printf("%.*s", (int)count, (char *)buf);
		//ALL THIS just to use printf? NOT ON MY WATCH!	
		//lets try to find out  how to fucking deal with writing directly to stdout
		/*me when the:*/ low_write(STDOUT_FILENO, buf, count); //funziona anche da sola LUL
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
		printf("Syscall fallita, return=%ld\n", result);
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
}




void print_string(const char *str) {
	while (*str) {
		my_write(STDOUT_FILENO, str, 1);
		str++;
	}
}

void print_float(double num, int precision) {
	if (num<0) {
		write(1, "-", 1);
		num=-num;
	}
	int int_part = (int)num;
	print_int(int_part);
	my_write(STDOUT_FILENO, ".", 1);
	
	double fraction = num -int_part;
	int fraction_as_int = (int)(fraction * power10(precision));
	print_int(fraction_as_int);
}

void print_double (double num, long int precision) {
	if (num < 0) {
		low_write(1, "-", 1);
		num = -num;
	}
	
	long int int_part = (long int)num;
	print_int(int_part);
	my_write(1, ".", 1);
//Let's write the decimal part:
	
	double fraction = num - int_part;
	
	long int fraction_as_int = (long int)(fraction * long_power10(precision));
		if (fraction_as_int < 0) fraction_as_int = -fraction_as_int;
	printf("%ld", fraction_as_int);
	print_long_int(fraction_as_int);

}

long int long_power10(long int n) {
	long int result = 1;
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


int getch(void) { //riscrivere con fz di output
	return (bufp > 0) ? getch_buf[--bufp] : my_getchar();
}

void ungetch(int c) {
	if (bufp>=BUFFER_SIZE)
	my_printf("Ungetch: Troppi caratteri\n");
	else 
		getch_buf[bufp++] = c;
}

void print_int(int n) {
	char buf[20];
	int i =0;

	if (n<0) {
		my_write(STDOUT_FILENO, "-", 1);
		n=-n;
	}
	do {
		buf[i++] = (n%10) + '0';
		n/=10;
	} while (n > 0);

	while (i--) {
		my_write(STDOUT_FILENO, &buf[i], 1);
	}
}

void print_long_int (long int n) {
	char buf[129];
	long int i=0;
	if (n<0) {
		my_write(STDOUT_FILENO, "-",1);
		n=-n;
	}
	do {
		buf[i++] = (n%10) + '0';
		n/=10;
	} while (n > 0);
	while (i--) {
		my_write(1, &buf[i], 1);
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
					//const char *l = p; *l++;
					p++;
					//printf("%s", l);
					switch (*p++) {
						case 'f':
							{
							double k = va_arg(args, double);
							print_double(k, 12);
							break;
							}
						case 'd':
							//puts("ld");
							long int k = va_arg(args, long int);
							print_long_int(k);
							break;

						default :
							my_write(STDOUT_FILENO,p, 1);
					
						}
					}
				
				default:
					  my_write(STDOUT_FILENO, p, 1);
			}
		} else if (*p == '\\') {
			switch (*p) {
				case 'n': {
					 my_write(STDOUT_FILENO, "\n", 1);
					  break;
					  }
				case 't': {
					  my_write(STDOUT_FILENO, "\t", 1);
					  break;
					  }
				case '\'': {
					  my_write(STDOUT_FILENO, "\'", 1);
					  break;
					  }

				default:
					  my_write(STDOUT_FILENO, "\\", 1);
					  my_write(STDOUT_FILENO, p, 1);
			
			}
		} 
		else {write(STDOUT_FILENO, p, 1);}
	}

va_end(args);
}

//A simple implementation of scanf() (Why do I do this to myself?)
//cetriolo+d non va... O meglio va solo all'inizio
int my_scanf1(const char *format, ...) {
	va_list args;
	va_start(args, format);

	char buff[BUFFER_SIZE];
	ssize_t readBytes;
	int i=0;
	readBytes= low_read(0, buff, sizeof(buff)-1);
	if (readBytes <= 0) {
		if (readBytes==0) {
			va_end(args);
			return EOF;
		}
		va_end(args);
		return -1;
	}
	buffer[readBytes]= '\0';
	for (const char *p = format; *p; *p++) {
		if (*p == '%') {
			p++;
			switch (*p) {
				case 'd': {
					  int *i = va_arg(args, int*);
					  *i = string_to_int(buff);
					  break;
					  }
				case 's': {
					  char *s = va_arg(args, char *);
					  
					  while (*buff && !isspace(*buff)) {*s++ = *buff;}
					  *s='\0';
					  
					  break;
					  }
				default:
					  va_end(args);
					  return -1;
			}
		
		} 
	}
	va_end(args);
	return 0;
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


//A second implementation of scanf, from GeeksForGeeks mostly here to get an idea...
//Comunque che grandi eh, implementano la propria scanf chiamando una versione deprecata di scanf: fscanf o vfscanf
//se vogliamo essere pedanti...

int my_scanf2(const char *str, ...) {
	    char token[100]; 
    int k = 0; 
  
    // initializing list pointer 
    va_list ptr; 
    va_start(ptr, str); 
  
    // parsing the formatted string 
    for (int i = 0; str[i] != '\0'; i++) { 
  
        // copying str to token 
        token[k++] = str[i]; 
  
        // When a format specifier of null character is 
        // found 
        if (str[i + 1] == '%' || str[i + 1] == '\0') { 
            token[k] = '\0'; 
            k = 0; 
  
            // processing token 
            char ch1 = token[1]; 
  
            // for integers 
            if (ch1 == 'i' || ch1 == 'd' || ch1 == 'u') { 
                fscanf(stdin, "%i", va_arg(ptr, int*)); 
            } 
  
            // for short ubt 
            else if (ch1 == 'h') { 
                fscanf(stdin, "%hi", va_arg(ptr, short*)); 
            } 
  
            // for characters 
            else if (ch1 == 'c') { 
                char c; 
  
                // using this loop to ignore some chars 
                while ((c = fgetc(stdin)) == '\n'
                       || c == ' ' || c == EOF) { 
                } 
                *va_arg(ptr, char*) = c; 
            } 
            // for float 
            else if (ch1 == 'f') { 
                fscanf(stdin, "%f", va_arg(ptr, float*)); 
            } 
            else if (ch1 == 'l') { 
                char ch2 = token[2]; 
  
                // for long int 
                if (ch2 == 'u' || ch2 == 'd'
                    || ch2 == 'i') { 
                    fscanf(stdin, "%li", 
                           va_arg(ptr, long*)); 
                } 
  
                // for double 
                else if (ch2 == 'f') { 
                    fscanf(stdin, "%lf", 
                           va_arg(ptr, double*)); 
                } 
            } 
            else if (ch1 == 'L') { 
                char ch2 = token[2]; 
  
                // for long int 
                if (ch2 == 'u' || ch2 == 'd'
                    || ch2 == 'i') { 
                    fscanf(stdin, "%Li", 
                           va_arg(ptr, long long*)); 
                } 
  
                // for long double 
                else if (ch2 == 'f') { 
                    fscanf(stdin, "%Lf", 
                           va_arg(ptr, long double*)); 
                } 
            } 
  
            // for string 
            else if (ch1 == 's') { 
                fscanf(stdin, "%s", va_arg(ptr, char*)); 
            } 
        } 
    } 
  
    // closing va_list 
    va_end(ptr); 
    return 0; 
} 









//TESTING
int main(void) {
	/*const char *msg ="Hello m\' people!\t The syscall has spoken!\n";
	my_write(STDOUT_FILENO, msg, strlen(msg));
	const char *msg2= "Fuck this shit, Imma run directly on the low!\n";
	low_write(1, msg2, strlen(msg2));	
	const char *msg3 = "Puts calls for aid";
	const char *msg4= "N";
	my_puts(msg3);
	my_native_putchar((int)*msg4);
	int c = my_getchar(); 
	my_putchar(c);*/
	int num; char str[4];
	my_scanf2("%d %s", &num, &str);
	
	
	int test= 5;	
	my_printf("\nSperando %f\t%d che funzioni... %ld\t%lf\n", 1.5, test, 123456789012, 15.1878123468173);
	printf("\nSperando %f\t%d che funzioni... %ld\t%.12lf\n", 1.5, test, 123456789012, 15.1878123468173);
	return 0;
}




