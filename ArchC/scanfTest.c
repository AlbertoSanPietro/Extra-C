#include <stdio.h>
#define BUF_SIZE 1024

int main(void) {
  int buf[BUF_SIZE] = {0};
  puts("Inserit something, Enter to close the buffer");
  scanf("%d", &buf);
  
  for(int i=0; i<BUF_SIZE; i++) {  
    if (buf[i] != 0) {printf("%d", buf[i]);}
    else {i++;}
  }


  return 0;
}
