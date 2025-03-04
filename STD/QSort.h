/*
 * =====================================================================================
 *
 *       Filename:  QSort.c
 *
 *    Description: A non-recursive quick sort algorithm. There are some qs versions
 *    that allow for O(1) in space complexity, but the study is unfortunately privated
 *
 *        Version:  1.0
 *        Created:  03/04/2025 02:34:29 PM
 *       Revision:  none
 *       Compiler:  gcc
 * =====================================================================================
 */

/*	A downside of quicksort is that the worst case scenario has quadratic complexity
 *	This leads to a deep stack depth and therefore high memory usage. One way to avoid
 *	this scenario is to replace the call stack with an explicit stack.
 *	Qs is a partition exchange sort, so we need a partitioning procedure.
 *	The procedure arranges an array based on a pivot element, with all 
 *	greater elements falling to its right, and all smaller elements to its
 *	left. The following partiton is Sedgewick's "crossing pointers", derived
 *	from Hoare's original partitioning algorithm.
 */
#include <stdio.h>
#include <stdlib.h>

void qs(int a[], int l, int r) {
	if (r>l) {
		int i=partition(a, l, r);
		qs(a, l, i-1);
		qs(a, i+1, r);
	}

}

int partition(int a[], int l, int r) {
	int i=l-1, j=r, v=a[r];
	for (;;) {
		while(a[++i] < v);
		while(a[--j]> v);
		if (i>=j) break;
		exch(*a, i, j);
	}
	exch(*a, i, r);
	return i;
}

/*	Removing recursion
 *	Every recursive algorithm has an iterative analog. It is a bit hard to reach 
 *	the analogous iterative algorithm for qsort because of how the l and r parameters
 *	are determined. We need a way to save them into the partition function. 
 *
 *	We can use a basic stack for this, either by using a library or by implementing it 
 *	ourselves with a ptr and an array.
 */

void iterQuick(int a[], int l, int r) {
	int *stack = malloc(((r-l) +1 ) * sizeof(int));
	int top= -1;
	stack[++top] = l;
	stack[++top]=r;

	while (top >=0) {
		r = stack[top--];
		l=stack[top--];
		int i = partition(a, l, r);
	 if (i-1>l) {
		stack[++top]=l;
		stack[++top]=i-1;
	}
	if (i+1 < r) {
		stack[++top] = i +1;
		stack[++top] = r;
		}
	}	
}

/*	There are still a few optimizations we can do. An obvious one is to push
 *	the smaller array to the stack first to sort the larger of the two subarrays first
 *
 *	In the above algorithm we are pushing 2 pairs of value to the stack and removing 
 *	one immediately. We can optimize this with a method similar to tail call 
 *	elimination for recursion: we replace pushing one of the pairs to the stack.
 *	We would use a loop to optimize out recursion. 
 *
 *	Optimizations that work for the recursive version also work: we can switch to 
 *	insertion sort for smaller arrays. Similarly proper pivot selection is fundamental
 *	Median of three and Median of Medians are all good ideas.
 *
 *	Below is an iterative quick sort with median of three, small array insertion
 *	sort and sorting larger sub arrays first with tail call elimination.
 */

const int CUTOFF = 6;

void insertionSort(int a[], int l, int r) {
	for (int i=l; i<=r; i++) {
		int j=i, v = a[i];
		while (j>l && a[j-1] > v) {
			a[j] = a[j-1];
			j--;
		}
		a[j] = v;
	}
}

void exch(int *a, int l, int r) {
	int tmp=a[l];
	a[l]=a[r];
	a[r]=tmp;
}

void medOf3(int a[], int l, int r) {
	puts("median of 3.");
	
	int m=(l+r)/2;
	if (a[l] > a[m]) exch(a, l, m);
	if (a[l] > a[r]) exch(a, l, r);
	if (a[m] > a[r]) exch(a, m, r);
	exch(a, m, r);
}

void push(int stack[], int *top, int l, int r) {
	stack[++(*top)]=l;
	stack[++(*top)]=r;
}

void iterQS(int a[], int l, int r) {
	int *stack = malloc(((l-r) +1) * sizeof(int));
	int top = -1;

	for(;;) {
		if (r -l < CUTOFF) {
			insertionSort(a, l, r);
		} else {
			while(r>l) {
				int i = partition(a, l, r);
				if ((i-1) -l < r - (i+1)) {
					l=i+1;
				} else {
					if (i+1<r) push(stack, &top, i+1, r);
					r=i-1;
				} 
			}
		}
		if (top<0) 
			break;
		r=stack[top--];
		l=stack[top--];
	}

}



