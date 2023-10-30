all: tinyem.sh tinyem
	./$<

tinyem: tinyem.c
	gcc -Wall -Wextra -pedantic -g3 $< -o $@
	sudo setcap CAP_NET_ADMIN+ep $@


clean:
	-rm tinyem

.PHONY: all clean
