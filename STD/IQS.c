#include <stdio.h>
#include <limits.h>  // for INT_MAX

// Swap helper
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// A simple insertion sort for A[1..n].
void insertSort(int A[], int n) {
    for (int k = 2; k <= n; k++) {
        int x = A[k];
        int j = k - 1;
        while (j >= 1 && A[j] > x) {
            A[j + 1] = A[j];
            j--;
        }
        A[j + 1] = x;
    }
}

// IQS: the O(1)-space version of Quicksort, using sentinels:
// A[n+1] = maxval - 1, A[n+2] = maxval (e.g., INT_MAX-1 and INT_MAX).
// Sorts A[1..n].
void IQS(int A[], int n) {
    int i, j, l, r, m;
    int p;  // pivot

    // Initialize
    l = 1;
    r = n + 1;  // points to A[n+1], the first sentinel
    m = 9;      // threshold for switching to insertion sort

    // The outer repeat..until in Pascal => do/while in C
    do {
        // While the current segment is large, partition
        while ((r - l) > m) {
            i = l;
            j = r;
            p = A[l];  // pivot (could also do median-of-three, etc.)

            // Partition loop (Hoare-style)
            while (1) {
                // Move i right until A[i] >= pivot
                do { i++; } while (A[i] < p);
                // Move j left until A[j] <= pivot
                do { j--; } while (A[j] > p);
                if (i < j) {
                    swap(&A[i], &A[j]);
                } else {
                    break;
                }
            }

            // Put pivot into correct position
            A[l] = A[j];
            A[j] = p;

            // Instead of pushing on a stack, swap with A[r]
            swap(&A[i], &A[r]);
            // Now shrink the right boundary
            r = j;
        }

        // Move left boundary up to r
        l = r;

        // Advance l until we find a value not equal to pivot p
        // (Pascal: "repeat l := l+1; until A[l] <> p;")
        do {
            l++;
        } while (A[l] == p);

        // If still within array, we do a sequential search for the new r
        // (Pascal code: "if l <= n then begin p := A[l]; r := l; ... end;")
        if (l <= n) {
            p = A[l];
            r = l;
            // Move r until we find A[r] > p
            do {
                r++;
            } while (A[r] <= p);
            r--;
            // Swap pivot into position
            A[l] = A[r];
            A[r] = p;
        }
    } while (l <= n);

    // Finish off with an insertion sort on A[1..n].
    insertSort(A, n);
}

// Demo main
int main(void) {
    // Example array size
    int n = 10;
    // We need space for A[1..n] plus A[n+1], A[n+2] for sentinels => total n+2.
    // In C, we typically do 0..n+1, but we won't use index 0 here.
    int A[13];  // Enough for indices 1..12

    // Fill A[1..n] with data
    A[1] = 34; A[2] = 7;  A[3] = 23; A[4] = 32; A[5] = 5;
    A[6] = 62; A[7] = 32; A[8] = 2;  A[9] = 12; A[10] = 45;

    // Set sentinels: A[n+1] = INT_MAX-1, A[n+2] = INT_MAX
    A[11] = INT_MAX - 1;  
    A[12] = INT_MAX;

    // Call IQS
    IQS(A, n);

    // Print result (A[1..n])
    for (int i = 1; i <= n; i++) {
        printf("%d ", A[i]);
    }
    printf("\n");

    return 0;
}

