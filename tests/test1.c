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


	while(n) {
		p = fork();	
		
		if(!p)  {
			n--;
		} else {
			if((r = wait_exit(p)) == -1)
				printf("DUPA: result: %d, errno: %d\n", r, errno);

			break;
		}
	}

	if(!n)
		sleep(3);

	printf("Process %d exiting\n", AMOUNT - n);

	return 0;
}
