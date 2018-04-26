#include <lib.h>
#include <minix/com.h>
#include <minix/ipc.h>
#include <minix/rs.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

#define AMOUNT 20

void handler(int sig){}

int main() {
	message m;
	endpoint_t ep;
	int n = AMOUNT;
	int r;
	pid_t p;

	struct sigaction act;
	act.sa_handler = handler;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGUSR1, &act, NULL);

	p = fork();

	if(!p) {
		sleep(2);
		kill(getppid(), SIGUSR1);
		printf("chld done\n");
		return 0;
	}

	
	if((r = wait_exit(1)) != 0) {
		printf("errno: %d\n", errno);
	}


	return 0;
}
