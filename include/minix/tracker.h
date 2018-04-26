#ifndef  tracker_INC
#define  tracker_INC

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  watch_exit
 *  Description:  When the specified process ends
 *  the caller of this function will recive a notify
 * =====================================================================================
 */
int watch_exit (endpoint_t);


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  cancel_watch_exit
 *  Description: Cancel the watch request 
 * =====================================================================================
 */
int cancel_watch_exit (endpoint_t ep);


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  query_exit
 *  Description: Writes id of a terminated, watched process to a pointer given as argument
 *  and returns the amount of currently watched, unfinished processes
 * =====================================================================================
 */
int query_exit (endpoint_t*);


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  wait_exit
 *  Description:  Blocking call that finishes after the specified process is terminated
 * =====================================================================================
 */
int wait_exit (pid_t);
#endif   /* ----- #ifndef tracker_INC  ----- */
