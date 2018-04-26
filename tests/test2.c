#include <lib.h>
#include <minix/com.h>
#include <minix/ipc.h>
#include <minix/rs.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define AMOUNT 20

int main() {
	message m;
	endpoint_t ep;
	int n = AMOUNT;
	int r;
	pid_t p;
	pid_t root;

	root = fork();

	if(!root) {
		sleep(5);
		return 0;
	}

	while(n) {
		p = fork();	
		
		if(!p)  {
			if((r = wait_exit(root)) == -1)
				printf("DUPA: result: %d, errno: %d\n", r, errno);
			
			break;
		} else  {
			n--;
		}
	}

	printf("Process %d exiting\n", AMOUNT - n);

	return 0;
}
