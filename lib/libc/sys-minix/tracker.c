#include <lib.h>
#include <minix/com.h>
#include <minix/ipc.h>
#include <minix/rs.h>
#include <stdio.h>
#include <stdbool.h>

int r;
message m;
endpoint_t dst;

void find_tracker() {
	if((r = minix_rs_lookup("tracker", &dst)) != 0) {
		printf("Failed to obtain tracker ep\n");
		dst = -1;
	}
}

int watch_exit(endpoint_t ep) {
	find_tracker();
	
	if(dst == -1)
		return -1;

	m.m1_i1 = ep;

	return _syscall(dst, 2, &m) == -1 ? -1 : 0;
}

int cancel_watch_exit(endpoint_t ep) {
	find_tracker();

	if(dst == -1)
		return -1;

	m.m1_i1 = ep;
	
	return _syscall(dst, 3, &m) == -1 ? -1 : 0;
}

int query_exit(endpoint_t* ep) {
	find_tracker();

	if(dst == -1)
		return -1;
	
	if((r = _syscall(dst, 4, &m)) == -1) {
		return -1;
	}

	*ep = m.m1_i1;

	return m.m1_i2;
}

int wait_exit(pid_t pid) {
	find_tracker();
	int status;

	if(dst == -1)
		return -1;	

	m.m1_i2 = pid;

	return _syscall(dst, 5, &m) == -1 ? -1 : 0;
}
