#define _POSIX_SOURCE 1
#define _MINIX 1
#define _SYSTEM 1

#include <lib.h>
#include <minix/sef.h>
#include <minix/config.h>
#include <stdlib.h>


#include "types.h"
#include "utility.h"

void init();
void fetch_process_info(endpoint_t ep, pid_t p);
void fetch_signal_info(endpoint_t ep);
int watch_exit(endpoint_t caller_ep, endpoint_t obs_ep);
void cancel_watch_exit(endpoint_t caller_ep, endpoint_t obs_ep);
int query_exit(endpoint_t caller_ep, endpoint_t* result);
int wait_exit(endpoint_t caller_ep, pid_t p);
void remove_client(int pos);
void remove_blocked(int pos);
void erase_list(Proc_list* list);

static Client obs_table[NR_PROCS];
static Blockedclient bl_table[NR_PROCS];
static int clientcount;
static int blockedcount;

int main() {
	init();
	message m;
	int result;

	while(1) {
		if((result = sef_receive(ANY, &m)) != 0) {
			printf("sef_receive failed");
			continue;		
		}
		switch(m.m_type) {
			/* Handle information from pm about terminated proc */
			case 0:
				/* If the message was not send by pm ignore it */
				if(m.m_source != PM_PROC_NR)
					break;

				fetch_process_info(m.m1_i1, m.m1_i2);
				break;

				/* Handlle information about pending signals from pm */
			case 1:
				if(m.m_source != PM_PROC_NR)
					break;

				fetch_signal_info(m.m1_i1);
				break;

				/* Handle watch_exit request */
			case 2:
				if(!is_systemproc(m.m_source)) {
					m.m_type = EPERM;
					break;
				}

				m.m_type = watch_exit(m.m_source, m.m1_i1);
				break;

				/* Handle cancel_watch_exit request */
			case 3:
				if(!is_systemproc(m.m_source)) {
					m.m_type = EPERM;
					break;
				}

				cancel_watch_exit(m.m_source, m.m1_i1);
				m.m_type = 0;
				break;

				/* Handle query_exit request */
			case 4:
				if(!is_systemproc(m.m_source)) {
					m.m_type = EPERM;
					break;
				}

				m.m1_i2 = query_exit(m.m_source, &(m.m1_i1));
				m.m_type = 0;
				break;

				/* Handle wait_exit request */
			case 5:
				m.m_type = wait_exit(m.m_source, m.m1_i2);
				/* If the process was blocked succesfully continue */
				if(m.m_type == 0)
					continue;

				break;

			default:
				printf("Unknown request type: %d\n", m.m_type);
		}
		/* Dont respond to messages from pm */
		if(m.m_source == PM_PROC_NR)
			continue;

		/* Send response by a non-blocking call,
		 * we assume that the caller is waiting for it */
		if(sendnb(m.m_source, &m) != 0)
			printf("Failed to send response \n");
	}
	return 0;
}

void fetch_process_info(endpoint_t ep, pid_t p) {
	message m;

	for(int i=0;i<clientcount;i++) {
		Proc_list* curr = obs_table[i].watched_list;

		/* One of our clinets just died */
		if(obs_table[i].id == ep) {
			printf("TR: Dead client\n");
			remove_client(i);
			i--;
			continue;
		}

		while(curr != NULL) {
			if(curr->id == ep) {
				printf("TR: Found one\n");
				curr->pending = true;
				if(notify(obs_table[i].id) != 0 && errno == ESRCH)
					remove_client(i);

				break;
			}
			curr = curr->next;
		}
	}

	for(int i=0;i<blockedcount;i++) {
		if(bl_table[i].id == ep) {
			printf("Blocked process died\n");
			remove_blocked(i);
			continue;
		}

		if(bl_table[i].watched_proc == p) {
			printf("Waking up %d\n", bl_table[i].id);
			if(sendnb(bl_table[i].id, &m))
				printf("Failed to wake up\n");

			remove_blocked(i);
			/* The entry under i was just swapped with the last one,
			 * process it again */
			i--;
		}
	}

}

void fetch_signal_info(endpoint_t ep) {
	message m;
	m.m_type = EINTR;
	
	for(int i=0;i<blockedcount;i++)
		if(bl_table[i].id == ep) {
			printf("Interruping %d\n", ep);
			if(sendnb(bl_table[i].id, &m))
				printf("sendnb failed\n");

			remove_blocked(i);
			return;
		}
}

int watch_exit(endpoint_t caller_ep, endpoint_t obs_ep) {
	bool done = false;
	Proc_list* curr;

	/* Check if obs_ep is pointing to a alive proc */
	if(!is_validep(obs_ep))
		return ESRCH;

	/* Scan for the caller in the clients list */
	for(int i=0;i<clientcount;i++) {
		if(obs_table[i].id == caller_ep) {
			curr = obs_table[i].watched_list;
			done = true;
			break;
		}
	}
	/* Too many requests */
	if(clientcount == NR_PROCS) {
		/* Set this to sth more appropriate? */
		return ENOMEM;
	}


	/* If the caller has none active requests,
	 * add him to the table */
	if(!done) {
		obs_table[clientcount].id = caller_ep;
		obs_table[clientcount].watched_list = malloc(sizeof(Proc_list));
		curr = obs_table[clientcount].watched_list;
		curr->next = NULL;
		curr->id = obs_ep;
		curr->pending = false;
		clientcount++;
		printf("TR: First observed proc added\n");
		/* Check if the requested proc is already being watched */
	}else while(curr != NULL) {
		if(curr->id == obs_ep)
			return 0;

		curr = curr->next;
	}

	printf("TR: watch_exit succesfull\n");
	/* Add the proc to the end of the list */
	curr = malloc(sizeof(Proc_list));
	curr->next = NULL;
	curr->id = obs_ep;
	curr->pending = false;

	return 0;
}

void cancel_watch_exit(endpoint_t caller_ep, endpoint_t obs_ep) {
	Proc_list* curr = NULL, *prev = NULL;
	int i=0;

	for(;i<clientcount;i++) {
		if(obs_table[i].id == caller_ep) {
			curr = obs_table[i].watched_list;
			break;
		}
	}

	if(curr == NULL) return;

	if(curr->id == obs_ep) {
		if(curr->next == NULL) {
			remove_client(i);
			return;
		}
		obs_table[i].watched_list = curr->next;
		free(curr);
		return;
	}

	prev = curr;
	curr = curr->next;

	while(curr != NULL) {
		if(curr->id == obs_ep) {
			prev->next = curr->next;
			free(curr);
			return;
		}
		prev = curr;
		curr = curr->next;
	}
	return;
}

int query_exit(endpoint_t caller_ep, endpoint_t* ep) {
	int i=0;
	Proc_list* curr = NULL, *prev = NULL;
	int result=0;

	/* Scan the table looking for the caller */
	for(;i<clientcount;i++) {
		if(obs_table[i].id == caller_ep) {
			curr = obs_table[i].watched_list;
			break;
		}
	}

	/* The caller is currently not watching anyone */
	if(curr == NULL) return 0;

	if(curr->pending) {
		*ep = curr->id;
		result++;

		if(curr->next == NULL) {
			printf("TR: Last observed fetched\n");
			remove_client(i);
			return 1;
		}
		
		obs_table[i].watched_list = curr->next;
		free(curr);
		curr = obs_table[i].watched_list;
	}

	prev = curr;
	curr = curr->next;

	while(curr != NULL) {
		if(curr->pending) {
			if(!result) {
				prev->next = curr->next;
				free(curr);
				curr = prev->next;
				result++;
				continue;
			}
			result++;	
		}
		prev = curr;
		curr = curr->next;
	}
	printf("TR: Query returning %d pending\n", result);

	return result-1;
}

int wait_exit(endpoint_t caller_ep, pid_t p) {
	if(!is_validpid(p))
		return ESRCH;	

	if(blockedcount == NR_PROCS)
		return ENOMEM;

	bl_table[blockedcount].id = caller_ep;
	bl_table[blockedcount].watched_proc = p;
	blockedcount++;
	return 0;
}

void remove_client(int pos) {
	erase_list(obs_table[pos].watched_list);
	obs_table[pos].id = obs_table[--clientcount].id;
	obs_table[pos].watched_list = obs_table[clientcount].watched_list;

	obs_table[clientcount].id = -1;
	obs_table[clientcount].watched_list = NULL;
}

void remove_blocked(int pos) {
	bl_table[pos].id = bl_table[--blockedcount].id;
	bl_table[pos].watched_proc = bl_table[blockedcount].watched_proc;
	bl_table[blockedcount].watched_proc = -1;
	bl_table[blockedcount].id = -1;
}

void erase_list(Proc_list* list) {
	if(list->next != NULL)
		erase_list(list->next);

	free(list);
}

void init() {
	sef_startup();
	blockedcount = 0;
	clientcount = 0;

	for(int i=0;i<NR_PROCS;i++) {
		obs_table[i].id = -1;
		obs_table[i].watched_list = NULL;
	}
}
