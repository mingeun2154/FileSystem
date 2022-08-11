test : test.o hw1.o hw2.o disk.o
	gcc -g test.o hw1.o hw2.o disk.o -o test
test.o : testcase.c
	gcc -g -c -o test.o testcase.c
hw1.o : hw1.c
	gcc -g -c -o hw1.o hw1.c
hw2.o : hw2.c
	gcc -g -c -o hw2.o hw2.c
disk.o : disk.c
	gcc -g -c -o disk.o disk.c

clean:
	rm *.o MY_DISK test
