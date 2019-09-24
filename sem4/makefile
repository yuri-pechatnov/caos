
GCC=arm-linux-gnueabi-gcc -marm
RUN=qemu-arm -L ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi

hello:
	${GCC} hello.c -o hello.exe
	${GCC} hello.c -S -o hello.S

hello_run: hello
	${RUN} ./hello.exe   
	
lib_sum:
	${GCC} lib_sum.c -c
	${GCC} lib_sum.c -O0 -S -o lib_sum_o0.E
	${GCC} lib_sum.c -O3 -S -o lib_sum_o3.E
	
my_lib:
	${GCC} -g my_lib_sum.S -c
	
my_lib_example: my_lib
	${GCC} -std=c99 -g my_lib_sum.o my_lib_example.c -o my_lib_example.exe
	
my_lib_example_run: my_lib_example
	${RUN} ./my_lib_example.exe 
	
my_lib_example_run_gdb: my_lib_example
	${RUN} -g 1234 ./my_lib_example.exe 
	# gdb-multiarch -q --nh   -ex 'set architecture arm'   -ex 'set sysroot ~/Downloads/sysroot-glibc-linaro-2.25-2018.05-arm-linux-gnueabi'   -ex 'file ./my_lib_example.exe'   -ex 'target remote localhost:1234'   -ex 'break main'   -ex continue   -ex 'layout split'

