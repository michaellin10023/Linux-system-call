#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <errno.h>
#include <pthread.h>

// #include <linux/dynamic_dump_stack.h>

#define INSDUMP_SYSCALL 359
#define RMDUMP_SYSCALL 360
struct dumpmode_t {
	unsigned int mode;
};

/*
	if dumpstack mode = 1
	Thread program to test if the process has same parent id 
	as the owner of dump stack and shares the program space
	stack trace will be printed
	
*/
void* open_file(void* input){
     int fd;
     printf("OPENING THE FILE\n");
     fd = open("/home/root/test", O_RDWR);
     if(close(fd)<0){
	printf("ERROR WHILE OPENING THE FILE\n");
     }
     return 0;
}


int main(int argc, char *argv[]) {
	pthread_t thread1;
	char symbol_name[40];
	struct dumpmode_t input;
	pid_t pid1;
	int ret = 0, fd;
	int dumpstackmode;
	int c;

	ret = 100;
	printf("Which case do you want to test?\n");
	printf("case 1: dump stack enable for any process\n");
	printf("case 2: dump stack enable for owner process if dump mode is 0\n");
	printf("case 3: dump stack enable for the processes that have same parent process as the owner if dumpmode is 1\n");
	scanf("%d", &c);
	switch (c)
	{
	case 1:
		
		dumpstackmode = 2;
		input.mode = dumpstackmode;
		snprintf(symbol_name, sizeof(char)*40, "%s", "sys_open");
		
		
		ret = syscall(INSDUMP_SYSCALL,symbol_name, &input);
		if(ret < 0){
			printf("ERROR %d\n", ret);
			return -1;
		}
		printf("DUMPSTACKID %d\n", ret);

		pid1 = fork();

		if(pid1 <= 0){
			printf("IN CHILD: OPENING FILE\n");
				fd = open("/home/root/script", O_RDWR);
				if(close(fd)<0){
				printf("ERROR WHILE OPENING THE FILE\n");
				}
			printf("TRYING TO REMOVE DUMPSTACKID %d\n", ret);

			
			if(syscall(RMDUMP_SYSCALL,ret)<0){
				printf("ERROR WHILE REMOVING THE DUMPSTACK\n");		
			}
		}
		break;
	case 2:
		dumpstackmode = 0;
		input.mode = dumpstackmode;
		snprintf(symbol_name, sizeof(char)*40, "%s", "sys_open");
		
		
		ret = syscall(INSDUMP_SYSCALL,symbol_name, &input);
		if(ret < 0){
			printf("ERROR %d\n", ret);
			return -1;
		}
		printf("DUMPSTACKID %d\n", ret);

		pid1 = fork();

		if(pid1 <= 0){
			printf("IN CHILD: OPENING FILE\n");
				fd = open("/home/root/script", O_RDWR);
				if(close(fd)<0){
				printf("ERROR WHILE OPENING THE FILE\n");
				}
			printf("TRYING TO REMOVE DUMPSTACKID %d\n", ret);

			
			if(syscall(RMDUMP_SYSCALL,ret)<0){
				printf("ERROR WHILE REMOVING THE DUMPSTACK\n");		
			}
		}
		break;
	case 3:
		dumpstackmode = 1;
		input.mode = dumpstackmode;
		snprintf(symbol_name, sizeof(char)*40, "%s", "sys_open");
		
		
		ret = syscall(INSDUMP_SYSCALL,symbol_name, &input);
		if(ret < 0){
			printf("ERROR %d\n", ret);
			return -1;
		}
		printf("DUMPSTACKID %d\n", ret);

		pid1 = fork();

		if(pid1 <= 0){
			printf("IN CHILD: OPENING FILE\n");
				fd = open("/home/root/script", O_RDWR);
				if(close(fd)<0){
				printf("ERROR WHILE OPENING THE FILE\n");
				}
			printf("TRYING TO REMOVE DUMPSTACKID %d\n", ret);

			
			if(syscall(RMDUMP_SYSCALL,ret)<0){
				printf("ERROR WHILE REMOVING THE DUMPSTACK\n");		
			}
		}
		pthread_create(&thread1,NULL, open_file, (void *) NULL);
		pthread_join(thread1,NULL);
		break;
	
	default:
		break;
	}

	
	return 1;
	
}