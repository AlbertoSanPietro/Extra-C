#ifdef __APPLE__
	#define _DARWIN_C_SOURCE
#elif defined(__linux__)
	#define _GNU_SOURCE
#endif

#include <csignal> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static jmp_buf env;

void sigHandler(int sig) {
	printf("Caught your signal %d \n", sig);
	//sleep(1);
	longjmp(env, 1);

}

void setSignals() {
	struct sigaction sa;
	sa.sa_handler=sigHandler;
	sa.sa_flags= SA_RESTART;
	sigemptyset(&sa.sa_mask);
	
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);	

	signal (SIGTSTP, SIG_IGN);
}

void reopen_stdin() {
	fclose(stdin);
	stdin=fopen("/dev/tty", "r");
}




int main() {
	char buffer[256];
	setSignals();

	if (setjmp(env)== 0){
		printf("exit if you can..");
	} else {
			puts("Reopened for input");
	}

	for(;;) {
		printf("> ");
		if (!fgets(buffer, sizeof(buffer), stdin)) {
			puts("EOF detected, try again");
			reopen_stdin();
		}

	}
}









