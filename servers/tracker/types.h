#ifndef  types_INC
#define  types_INC
#include <minix/type.h>
#include <stdbool.h>
struct proc_list {
	struct proc_list* next;
	endpoint_t id;
	/* Is observed process dead */
	bool pending;
};				/* ----------  end of struct proc_list  ---------- */

typedef struct proc_list Proc_list;

struct client {
	endpoint_t id;
	Proc_list* watched_list;
};				/* ----------  end of struct client  ---------- */

typedef struct client Client;


struct blockedclient {
	endpoint_t id;
	pid_t watched_proc;
};				/* ----------  end of struct blockedclient  ---------- */

typedef struct blockedclient Blockedclient;
#endif   /* ----- #ifndef types_INC  ----- */
