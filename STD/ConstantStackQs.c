/* In this version of the qsort the stack operations are replaced by the repeated search for bounds of the next unsorted subset. This should slow down operations
 * by only 4-8%. 
 * Qs is a great example of divide and conquer method: in the decomposition step we choose a pivot p between the elements of set S. S is then rearranged into S1 and S2, greater
 * and smaller than p. For time and space efficiency we have removed recursion and instead used a stack. But the stack is unneccesary. We cxan search for bounds in the next 
 * subproblem instead of storing them. 
 * In Qsort after the decomposition left and right bounds of S2 are pushed on the stack and S1 is dealt with further. here we instead	interchange the first element of S2 with
 * with the pivot p', splitting set S and S'. 
 * Now p' is the first element of S2 and it is greater or equal to all elements of set S2. 
 * The decomposition of S1 follows.
 * After decomposing S1 instead of popping of the bounds from the stack we:
 * A) Choose the left-hand side bound of S2 (behind p) to separate S1 and S2. The first element not equal to p' (in S2) is searched.
 * B)The first element greater than p' is searched to the right (this element belongs to set S' and NOT S2)
 * C) The 'borrowed' pivot p' will be returned into the original place between S2 and S' 
 * When the right hand side bound of S2 is found we can decompose S2 further. The algorithm will finish when the chosen left-hand side bound is 
 * 'behind' (right to) the original set S.
 * 
 * We now show in PASCAL (translated into C) how to write the algorithm. Insertion sort and swap are not included in this code, as they are well known. 
 *
 * For simplicity we start with a simpler stack-based QS:
 */

#include <stdio.h>

// Swap helper function.
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Insertion sort used at the end to finish sorting nearly sorted segments.
void insertionSort(int A[], int n) {
    int i, j, key;
    for (i = 1; i < n; i++) {
        key = A[i];
        j = i - 1;
        while (j >= 0 && A[j] > key) {
            A[j + 1] = A[j];
            j--;
        }
        A[j + 1] = key;
    }
}

// SQS: the stack-based quicksort procedure.
// It sorts the array A[0 .. n-1] using a quicksort partitioning
// method with a threshold (m) at which insertion sort is used.
void SQS(int A[], int n) {
    int stack[n];  // Use n elements as maximum stack size.
    int s = 0;     // Stack pointer.
    int l = 0, r = n; // Our subarray is A[l]..A[r-1]. (Initially, the whole array.)
    int m = 9;     // Threshold for switching to insertion sort.

    // Process subarrays until we cover the entire array.
    while (l < n) {
        // While the current subarray is larger than m, partition it.
        while (r - l > m) {
            int i = l;
            int j = r;
            int p = A[l];  // Use the first element as pivot.
            // Partitioning loop.
            while (1) {
                // Increase i until A[i] >= pivot.
                do {
                    i++;
                } while (i < r && A[i] < p);

                // Decrease j until A[j] <= pivot.
                do {
                    j--;
                } while (j > l && A[j] > p);

                if (i >= j)
                    break;
                swap(&A[i], &A[j]);
            }
            // Place pivot in its correct position.
            A[l] = A[j];
            A[j] = p;
            // Save the right bound on the stack.
            stack[s++] = r;
            // Now work on the left subarray.
            r = j;
        }
        // Move to the next subarray.
        l = r;
        if (l < n) {
            // Restore the right bound from the stack.
            r = stack[--s];
        }
    }
    // Finally, use insertion sort to finish.
    insertionSort(A, n);
}

// A simple main function to demonstrate the usage.
int main(void) {
    int A[] = { 34, 7, 23, 32, 5, 62, 32, 2, 12, 45, 18 };
    int n = sizeof(A) / sizeof(A[0]);
    int i;

    SQS(A, n);

    printf("Sorted array:\n");
    for (i = 0; i < n; i++) {
        printf("%d ", A[i]);
    }
    printf("\n");

    return 0;
}
//We now presente the IQSORT, or the sorting algorithm in constant stack. It is still in PASCAL (translated into C)
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
/*  int main(void) {
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
*/

/*  The search for bounds. The linear search implemented above is simple, but also very inefficient. Since the search of the right bound is opposite to the change of j in 
 *  the inner loop we consume half the time. Empirically we know 60% of the time in qsort is spent partitioning. Thus the equivalent O(1) space version 
 *  should be slower by 30%. Using the model 2 in ASM we see we actually beat HeapSort, which is also O(1). 
 *
 *  We can also speed up out algorithm by abandoning linear search: we can remove the redundant search  for the right hand-side of the subarray. Another optimization
 *  is to use a sequential search with step s>1 (s=10 for n=1000, s=12 for n=10000). After some checks in C and PASCAL we see that this way this qsort is only 4-8% slower
 *  with n=1000-10000. 
 *
 */

#include <stdio.h>
#include <limits.h>  // for INT_MAX

// Helper swap function.
void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

// Insertion sort for A[1..n].
void insertSort(int A[], int n) {
    for (int k = 2; k <= n; k++) {
        int temp = A[k];
        int j = k - 1;
        while (j >= 1 && A[j] > temp) {
            A[j + 1] = A[j];
            j--;
        }
        A[j + 1] = temp;
    }
}

// IQS: O(1)-space quicksort with improved bound searches.
// The array A is assumed to hold valid data in indices 1..n,
// with sentinels at A[n+1] (set to maxval-1) and A[n+2] (set to maxval).
void IQS(int A[], int n) {
    int i, j, l, r, m;
    int p;  // pivot value
    // s_step: the jump size used in the improved bound search.
    // For instance, s_step=50 works well for n=1000, adjust as needed.
    int s_step = 50;  

    // Initialization.
    l = 1;
    r = n + 1;  // A[n+1] is a sentinel
    m = 9;      // threshold for switching to insertion sort

    // Outer loop: continue until l > n.
    do {
        // Partition as long as the current segment is large.
        while ((r - l) > m) {
            i = l;
            j = r;
            p = A[l];  // Choose pivot (could also be improved to median-of-three).

            // Partitioning inner loop (Hoare-style).
            do {
                do {
                    i++;
                } while (A[i] < p);
                do {
                    j--;
                } while (A[j] > p);
                if (i < j)
                    swap(&A[i], &A[j]);
            } while (i != j);

            // Place pivot into its correct position.
            A[l] = A[j];
            A[j] = p;

            // --- Modification A ---
            // Instead of always swapping with A[r], check the gap.
            if ((j - l) <= m) {
                l = i;
            } else {
                swap(&A[i], &A[r]);
                r = j;
            }
        }

        // Now, the subarray is small. Adjust l to find a new partition.
        l = r;
        // Advance l until we find a value different from pivot p.
        do {
            l++;
        } while (A[l] == p);

        // If we are not done, use an improved search for the new right bound.
        if (l <= n) {
            p = A[l];
            r = l;
            // --- Modification B ---
            // Instead of a simple sequential search, first check using a jump.
            if (A[n - s_step] > p) {
                do {
                    r += s_step;
                } while (r <= n && A[r] < p);
                r = r - s_step;
            } else {
                r = n - s_step;
            }
            // Refine the bound with a sequential search.
            do {
                r++;
            } while (r <= n && A[r] <= p);
            r--;  // r now points to the last element ≤ p.

            // Swap pivot into its correct position.
            A[l] = A[r];
            A[r] = p;
        }
    } while (l <= n);

    // Finish sorting with an insertion sort.
    insertSort(A, n);
}

// Demo main function.
int main(void) {
    // Example: an array of n elements stored in A[1..n] with two sentinels.
    int n = 10;
    // Array size must be n+2; index 0 is unused.
    int A[13];

    // Fill A[1..10] with example data.
    A[1] = 34;  A[2] = 7;   A[3] = 23;  A[4] = 32;  A[5] = 5;
    A[6] = 62;  A[7] = 32;  A[8] = 2;   A[9] = 12;  A[10] = 45;
    // Set sentinels: A[n+1] = maxval-1 and A[n+2] = maxval.
    A[11] = INT_MAX - 1;
    A[12] = INT_MAX;

    // Call the improved IQS procedure.
    IQS(A, n);

    // Print the sorted array.
    for (int i = 1; i <= n; i++) {
        printf("%d ", A[i]);
    }
    printf("\n");

    return 0;
}






