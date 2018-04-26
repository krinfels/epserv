#include <lib.h>
#include <minix/com.h>
#include <minix/ipc.h>
#include <minix/rs.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

int main() {
	if(watch_exit(1) == -1 && errno == EPERM) {
		printf("OK\n");
		return 0;
	}

	printf("NOPE\n");
	return 0;
}
