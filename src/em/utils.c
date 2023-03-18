#include <stdio.h>
#include <stdint.h>

#include "utils.h"


void dump(const char *buf, size_t len) {
	size_t i, j;

	for (i = 0; i < len; i++) {
		if ((i % 8) == 0)
			printf("%04hx  ", (uint16_t) i);

		printf("%02hhx", buf[i]);
		if ((i % 8) == 3) {
			printf("  ");
		} else if ((i % 8) == 7) {
			printf("  ");
			for (j = i - 7; j <= i; j++)
				if ((buf[j] < 32) || (buf[j] > 126))
					printf(".");
				else
					printf("%c", buf[j]);
			printf("\n");
		} else {
			printf(" ");
		}
	}

	if ((i % 8) != 0) {
		for (j = i % 8; j < 8; j++) {
			printf("  ");
			if (j == 3)
				printf("  ");
			else
				printf(" ");
		}
		printf(" ");
		for (j = i - (i % 8); j < i; j++)
			if ((buf[j] < 32) || (buf[j] > 126))
				printf(".");
			else
				printf("%c", buf[j]);
		printf("\n");
	}
}