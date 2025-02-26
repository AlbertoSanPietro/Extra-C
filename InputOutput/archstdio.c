/*"minimal" rewrite of stdio.h to better understand some inner workings of it.
 *The code here is based on the glibc, but it is very much simplified and les robust
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
#include <ctype.h>
#include <wctype.h>
#include <wchar.h>
#include <stdbool.h>
#include <limits.h>



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



//Some necessary defines
#define BUFFER_SIZE 1024
#define SYS_write 1
#define SYS_read 0
#define INT_MAX_LAST_DIGIT (INT_MAX % 10) //One of the two possible approaches
                                          //to optimization

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
void print_long_int(long int);

//Getch and Ungetch...
int getch(void);
void ungetch(int);
/*******************************************************************************************/
//INPUT functions (stdin)
int my_getchar(void);
ssize_t bytes;
ssize_t low_read(int, const void *, size_t);
int my_scanf1(const char *, ...);
int my_scanf2(const char *, ...);
int my_scanf3(const char *, ...);
void scan_int(int);

/************************************************/
//Helper functions
int power10(int);
long int long_power10(long int);
int string_to_int(const char *);
void enable_raw_mode();
void disable_raw_mode();


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
		 low_write(STDOUT_FILENO, buf, count);
		 //my_write was a first approach at the write() function. The low_write is the actual implementation
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


int getch(void) { //not operative as of yet
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
							long int k = va_arg(args, long int); //This is seen by clangd plugin as a C23 extensions unfortunately.
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

/****************************************************************/
//THIS PART IS NECESSARY FOR SCANF() TO WORK AS IT SWITCHES THE TERMINAL MODE USING <termios.h>
//OR 
//So I thought. In reality this is completely useless and only here because I found it interesting, but it has no bearing and it is never called
//nor referenced
void enable_raw_mode() {
	struct termios term; puts("raw_struct");
	tcgetattr(STDIN_FILENO, &term); puts("raw_tcgetattr");
	struct termios raw = term;
	raw.c_lflag &= ~(ICANON | ECHO); //Wtf am I even doing here...
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() {
	struct termios term;
	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag |= ICANON | ECHO;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}



//A simple implementation of scanf() That does now work. This was the first attempt, following attempts have a number near them...
int my_scanf1(const char *format, ...) {
	va_list args;
	va_start(args, format);

	char buff[BUFFER_SIZE];
	int buffIndex=0;
	ssize_t readBytes;
	int numAssigned = 0; //Flag, keeps track of succesfull inputs
	int i=0;
	char c;
	int input_complete = 0;
	enable_raw_mode();
	//while(!input_complete) {
	while((readBytes = low_read(STDIN_FILENO, &c, 1)) > 0) {
		buff[readBytes] = '\0';	
	//while ((readBytes= low_read(0, buff, sizeof(buff)-1)) > 0 ){
	//buffer[readBytes]= '\0';
	
	for (const char *p = format; *p; *p++) {
		if (*p == '%') {
			p++;
			switch (*p) {
				case 'd': {
					  puts("Sono dentro lo switch d"); 
					  int *i = va_arg(args, int*);
					  *i = string_to_int(buff);
					  numAssigned++;
					  break;
					  }
				case 's': {
					  char *s = va_arg(args, char *);
					  
					  while (*buff && !isspace(*buff)) {*s++ = *buff;}
					  *s='\0';
					  numAssigned++;
					  break;
					  }
				default:
					  va_end(args);
					  disable_raw_mode();
					  return -1;
				}
		
			} 
		}
		continue;
	} /*else {
		if (buffIndex < BUFFER_SIZE -1) {
			buff[buffIndex++] = c;}
	}*/

	if (readBytes == 0) {
		va_end(args);
		disable_raw_mode();
		return (numAssigned == 0) ? EOF : numAssigned;
	}
	if (readBytes < 0) {
		va_end(args);
		disable_raw_mode();
		return -1;
	}

	va_end(args);
	disable_raw_mode;
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
//This is the unfortunate version provided by GeeksForGeeks. It uses fscanf instead, not what I was looking for at all...
/*int my_scanf2(const char *str, ...) {
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
                //fscanf(stdin, "%i", va_arg(ptr, int*));//we could use directly
                                                       //a syscall?
                low_read(STDIN_FILENO, va_arg(ptr, int*), 100);
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
                } void print_int(int n) {
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
}*/ 

//A third implementation of scanf... this broke me for some time. I had to come back a few weeks later and finally the next version kinda works
int my_scanf3(const char *format, ...) {
	va_list args;
	va_start(args, format);
	/*
	for (const char *p = format; *p; p++){
		if (*p=='%') {
			switch (*p) {
				case ('d'): {
					    int c = 0, i=0;
					    char buff[BUFFER_SIZE];
					    while ((c=my_getchar())!=EOF && i<BUFFER_SIZE ) {
					    	buff[i++] = c;
					    	}
					    buff[++i] = '\0';
					    return 0;
					    }
				default :
					    return 1;
			}
		}
	}
	*/
	int c=0, i=0;
	char buff[BUFFER_SIZE];
	while ((c=getchar()) != EOF && i < BUFFER_SIZE) {buff[i++]=c;}
	buff[i+1]='\0';
	va_end(args);
	return i;
}

/*
int my_scanf4(const char *format, ...) {
  va_list arg;
  int done;

  va_start(arg, format);
  done = __vscanf_internal (stdin, format, arg, 0);
  va_end(arg);
  return done;
}
*/

int process_int(const char **format, char *input, va_list *args) {
  int *num = va_arg(*args, int *);
  int64_t value = 0;
  int sign = 1;

  //skip spaces
  while (isspace(*input)){ input++;}

  //check sign
  if (*input == '-') {sign = -1; input++;}
  const int MAX_SAFE=INT_MAX/10;

  //convert to integer
  while (*input && isdigit(*input)) {
    //check for overflow, the update value
    if (value > MAX_SAFE ||(value == MAX_SAFE && (*input -'0') > ((double)INT_MAX) % 10)) 
      {perror("Scanf4: EINVAL: buffer overflow"); return -1;}
    value = value * 10 + (*input - '0');
    input++;
  }
  *num = value * sign;
  
  return 1; //SUCCESS!
}
//This version of scanf actually works, provided the number isn't too big. It overflows very easily despite my attempts at controlling overflow
//It only receives int and long (although long is somewhat broken)
//I will add other types and solve the OF issues in the future
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




//TESTING: the main function here only contains tests. It is chaotic as it contains almost every test I have run to check the correct working of every function.
//The messages here can contain crass language as the library was originally born to be experimented with by some friends of mine and only by them.

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

  /*
	int num; char str[4]; 
	my_scanf1("%d", &num);
	disable_raw_mode();
	puts("return");
	
	printf("Scanf: %d\t", num);	
	for (int i=0; i<4; i++) printf("Scanf2: %s\t", str[i]);
	*/
    
    //int scanf3; my_scanf3("%d", &scanf3); printf("scanf3: %d\n", scanf3);
   
/*    int64_t scanf4; my_scanf4("%d", &scanf4);
    int i=0;
    printf("scanf4: %d\n", scanf4);
	int test= 5;	
	//my_printf("\nSperando %f\t%d che funzioni... %ld\t%lf\n", 1.5, test, 123456789012, 15.1878123468173);
	//printf("\nSperando %f\t%d che funzioni... %ld\t%.12lf\n", 1.5, test, 123456789012, 15.1878123468173);
	*/
  return 0;
}




