/* This is the "source" file for weird.s 
 * The idea was to simulate UB in older compiler that stored string literals in .data 
 * instead of .rodata
 * The code cannot be compiled with modern gcc/clang on Ubuntu Linux
 * Instead the assembly code itself has to be modified
 *
 *
 */ 

#include <stdio.h>

int weird() {
    "b"[0]= 'a';    // UB
    puts("b");
    return 0;
}

int main() {
    return weird();
}
