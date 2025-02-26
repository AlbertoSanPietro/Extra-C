#include <stdio.h>
#include <math.h>

int scanf_f(int A[], int n) {
  int i, number=0, j=0;

  for (i=n-1; i>=0; i--) {
    number+=pow(10, j)*A[j];
    j++;
  }

  return number;
}

int main(void) {
  int A[5],i=0;

  while (i<5) {
    scanf("%d", &A[i]);
    getchar();
    i++;

  }
  int number;
  number=scanf_f(A, i);
  printf("%d", number);

  return 0;

}
