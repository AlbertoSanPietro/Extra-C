#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>

#include <fcntl.h> /* O_ constants */
#include <unistd.h> /* ftruncate */
#include <sys/mman.h> /* mmap */

void pippo();
void pluto();
void foo(int *);


jmp_buf env;
void sigHandler(int sig) {
    switch (sig) {
        case SIGSEGV:
            printf("Caught SIGSEGV (Segmentation Fault). Recovering...\n");
            break;
        case SIGBUS:
            printf("Caught SIGBUS (Bus Error). Recovering...\n");
            break;
        case SIGILL:
            printf("Caught SIGILL (Illegal Instruction). Recovering...\n");
            break;
        default:
            printf("Caught signal %d. Recovering...\n", sig);
    }
    
    //printf("Caught signal %d (%s). Recovering...\n", sig, sig == SIGSEGV ? "Segmentation Fault" : "Unknown");
    longjmp(env, 1);
}


void foo(int *p) {
    p=NULL;
    *p=42;
}

void pippo() {

    printf("Attempting to cause Bus Error...\n");
    int fd;
    int *map;
    int size = sizeof(int);
    char *name = "/a";

    shm_unlink(name);
    fd = shm_open(name, O_RDWR | O_CREAT, (mode_t)0600);
    /* THIS is the cause of the problem. */
    /*ftruncate(fd, size);*/
    map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    /* This is what generates the SIGBUS. */
    *map = 0;        
}


__attribute__((noinline)) void illegal_instruction() {
    __asm__ volatile ("ud2"); //GNU __asm__ macro and volatile to avoid optimizaziton
}

void pluto() {
    printf("Attempting to execute an illegal instruction...\n");
    void (*func)() = illegal_instruction;
    func();  // Call function pointer (compiler is less likely to optimize it away)
}

void input() {

}

int main()
{
    printf("Hello world!\n");
   int *p=malloc(sizeof(int));
   signal(SIGSEGV, sigHandler);
    signal(SIGBUS, sigHandler);
    signal(SIGILL, sigHandler);
   signal(SIGINT, sigHandler);
    if (setjmp(env) ==0){
     foo(p); 
   }
   
   if (setjmp(env) == 0) {
        pippo();
    } 

    if (setjmp(env) == 0) {
        pluto();
    } 

    if(setjmp(env) != 0) {
        input();
    }
   
   return 0;
}
