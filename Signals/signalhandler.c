/*
 * =====================================================================================
 *
 *       Filename:  signalhandler.c
 *
 *    Description: A signal handler to catch the segv signal 
 *
 *        Version:  1.0
 *        Created:  03/03/2025 04:45:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>


void handler (int s) {
	if (s==SIGBUS) printf("got a bus error signal\n");

	if (s==SIGSEGV) printf("Got a segmentation fault\n");

	if (s==SIGILL) printf("Illegal instruction signal\n");
	exit(EXIT_FAILURE);
}
int main() {
	int *p=NULL;
	signal(SIGBUS, handler);
	signal(SIGSEGV, handler);
	signal(SIGILL, handler);
	*p=0;
}

