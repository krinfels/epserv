#include <lib.h>
#include <minix/syslib.h>
#include <minix/callnr.h>
#include <minix/com.h>
#include <stdbool.h>

/* Ask PM if given ep represents a vaild proc */
bool is_validep(endpoint_t ep) {
	message m;
	int r;

	m.PM_ENDPT = ep;

	if((r = _taskcall(PM_PROC_NR, GETEPINFO, &m)) < 0)
			return false;

	return true;
}

bool is_systemproc(endpoint_t ep) {
	message m;
	int r;
	m.PM_ENDPT = ep;

	if((r = _taskcall(PM_PROC_NR, 110, &m)) < 0)
		return false;

	/* m2_i1 == response */
	return m.m2_i1;
}

bool is_validpid(pid_t p) {
	message m;
	int r;
	/* m1_i1 == pid in this call */
	m.m1_i1 = p;

	if((r = _taskcall(PM_PROC_NR, GETPROCNR, &m)) < 0) {
		printf("is_validpid %d failed with %d\n", p, r);
		return false;
	}

	return m.PM_ENDPT > 0;
}
