Directory contains a patch file, two test programs and a Makefile for it. 

How to build kernel : 

1. copy patch file
2. run patch -p1 < <patch.file>
3. build kernel
4. copy bzImage to board and reboot

How to run test program :

1. make
2. scp file(insdump_test and rmdump_test) to board
3. ./insdump_test  
	This program will test the three case describe in the assignment
4. ./insdump_test
	This program will test to remove dump stack

