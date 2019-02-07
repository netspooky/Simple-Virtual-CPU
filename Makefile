release:
	gcc -g -Wall -std=c11 -o cpu cpu.c

debug:
	gcc -g -Wall -std=c11 -o cpu cpu.c -DDEBUG
