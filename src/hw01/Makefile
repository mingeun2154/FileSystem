hw1: testcase.c validate.o fs.o disk.o
	gcc -o hw1 testcase.c validate.o fs.c disk.c

fs.o: fs.c 
	gcc -c -o fs.o fs.c

disk.o: disk.c
	gcc -c -o disk.o disk.c

clean: 
	rm hw1 fs.o disk.o