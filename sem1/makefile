all: build
	gdb -ex=r --args ./program_a
	#./program_a

build:
	gcc -c lib.c -o lib.o -g -O0
	gcc -DDEBUG lib.o heloo.c -g -O0 -o program_a

preprocessing:
	gcc heloo.c -E -o heloo.E
