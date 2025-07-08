//The following 
#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h> // for testing only

typedef unsigned char byte;
typedef unsigned long int op_t;
#define OPSIZ (sizeof(op_t))

void my_strcpy(char *, const char*);
void my_strncpy(char *, const char *, size_t);
size_t my_strlcpy(char *, const char *, size_t);

void *my_short_memset(void *, int, size_t);
void *my_long_memset(void *, int, size_t);

/***********************************************************************/

// Basic strcpy implementation
void my_strcpy(char *to, const char *from) {
	while((*to++ = *from++)) ;
}

// strncpy implementation
void my_strncpy(char *to, const char *from, size_t n) {
	size_t i;
	for (i=0; i<n && *from != '\0'; i++) {
		*to++=*from++;
	}
	for (; i<n; i++ ) {
		*to++='\0';
	}
}

// strlcpy-like implementation
size_t my_strlcpy(char *to, const char *from, size_t size) {
	size_t src_len =0;
	const char *src = from;
	while(*src++) {src_len++;}

	if (size > 0) {
		size_t copy_len = (src_len >=size)? (size-1) : (src_len);
		for(size_t i =0; i < copy_len; i++) {
			to[i]=from[i];
		}
		to[copy_len] = '\0';
	}
	return src_len;
}

// Optimized memset: aligns to word size and sets memory in word-sized chunks
void *my_short_memset(void *dest, int val, size_t len) {
	unsigned char byte_val = (unsigned char)val;
	unsigned char *dest_byte = (unsigned char *)dest;

	// Align destination pointer to word boundary
	while (((unsigned long)dest_byte % sizeof(op_t)) != 0 && len > 0) {
		*dest_byte++ = byte_val;
		len--;
	}

	// Fill a word with repeated byte_val using bitwise OR and shifts
	op_t word_val = byte_val;
	word_val |= word_val << 8;
	word_val |= word_val << 16;
#if __SIZEOF_LONG__ == 8
	word_val |= word_val << 32;
#endif

	op_t *dest_word = (op_t *)dest_byte;
	while (len >= sizeof(op_t)) {
		*dest_word++ = word_val;
		len -=sizeof(op_t);
	}

	dest_byte = (unsigned char *)dest_word;
	while (len >0) {
		*dest_byte++= byte_val;
		len--;
	}

	return dest;
}

// Aggressive memset with glibc-style loop unrolling
void *my_long_memset(void *dstpp, int c, size_t len) {
	//long int dstp = (long int) dstpp;
	uintptr_t dstp = (uintptr_t) dstpp;
	if (len >= 8) {
		size_t xlen;
		op_t cccc = (unsigned char) c;
		cccc |= cccc << 8;
		cccc |= cccc << 16;
		if (OPSIZ > 4) {
			cccc |= (cccc << 16) << 16;
		}

		// Align pointer
		while (dstp % OPSIZ != 0) {
			((byte *) dstp)[0] = c;
			dstp += 1;
			len -= 1;
		}

		// Unrolled loop: write 8 * op_t
		xlen = len / (OPSIZ * 8);
		while (xlen > 0) {
			((op_t *) dstp)[0] = cccc;
			((op_t *) dstp)[1] = cccc;
			((op_t *) dstp)[2] = cccc;
			((op_t *) dstp)[3] = cccc;
			((op_t *) dstp)[4] = cccc;
			((op_t *) dstp)[5] = cccc;
			((op_t *) dstp)[6] = cccc;
			((op_t *) dstp)[7] = cccc;
			dstp += 8 * OPSIZ;
			xlen--;
		}

		len %= OPSIZ * 8;

		// Write one word at a time
		xlen = len / OPSIZ;
		while (xlen > 0) {
			((op_t *) dstp)[0] = cccc;
			dstp += OPSIZ;
			xlen--;
		}

		len %= OPSIZ;
	}

	// Write remaining bytes
	while (len > 0) {
		((byte *) dstp)[0] = c;
		dstp += 1;
		len -= 1;
	}
	return dstpp;
}

/*************************** TEST FUNCTIONS ***************************/

void test_memset() {
	size_t size = 32;
	char *buffer1 = (char *)malloc(size);
	char *buffer2 = (char *)malloc(size);
	char *buffer3 = (char *)malloc(size);
	memset(buffer1, 0xAA, size);
	my_long_memset(buffer2, 0xAA, size);
	my_short_memset(buffer3, 0xAA, size);

	int success = 1;
	for (size_t i = 0; i < size; i++) {
		if (buffer1[i] != buffer2[i]) {
			success = 0;
			printf("Difference at %zu: %x != %x\n", i, (unsigned char)buffer1[i], (unsigned char)buffer2[i]);
		}
	}

	int success2 = 1;
	for (size_t i = 0; i < size; i++) {
		if (buffer1[i] != buffer3[i]) {
			success2 = 0;
			printf("Buffer 3: Difference at %zu: %x != %x\n", i, (unsigned char)buffer1[i], (unsigned char)buffer3[i]);
		}
	}

	if (success) {
		printf("Buffer2: Test passed.\n");
	} else {
		printf("Buffer2: Test failed.\n");
	}

	if (success2) {
		printf("Buffer3: Test passed.\n");
	} else {
		printf("Buffer3: Test failed.\n");
	}

	free(buffer1);
	free(buffer2);
	free(buffer3);
}

/******************************* MAIN *********************************/

int main(void) {
	char *test_strcpy =(char *)malloc(20), 
	     *test_strncpy =(char *)malloc(20), 
	     *strncpy_truncation =(char *)malloc(15), 
	     *origin="Prova funzionamento";

	my_strcpy(test_strcpy, origin);
	printf("%s\n", test_strcpy);

	size_t origin_len = 19;
	my_strncpy(test_strncpy, origin, origin_len);
	printf("%s\n", test_strncpy);

	my_strlcpy(strncpy_truncation, origin, 15);
	printf("%s\n", strncpy_truncation);

	test_memset();

	return 0;
}

