test : test.o hw1.o hw2.o disk.o
	gcc test.o hw1.o hw2.o disk.o -o test
test.o : testcase.c
	gcc -c -o test.o testcase.c
hw1.o : hw1.c
	gcc -c -o hw1.o hw1.c
hw2.o : hw2.c
	gcc -c -o hw2.o hw2.c
disk.o : disk.c
	gcc -c -o disk.o disk.c

clean:
	rm *.o MY_DISK test
debug:
	gcc -g test.o hw1.o hw2.o disk.o -o debug
