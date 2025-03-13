#ifndef EVQ_SELECT_H
#define EVQ_SELECT_H

#define EVQ_SOURCE	"select.c"

#define EVENT_EXTRA							\
  struct event_queue *evq;						\
  unsigned int index;

#define EVQ_EXTRA							\
  struct timeout_queue *tq;						\
  pthread_mutex_t sig_cs;						\
  int volatile sig_ready;  /* triggered signals */			\
  fd_t sig_fd[2];  /* pipe to interrupt the loop */			\
  unsigned int npolls, max_fd;						\
  struct event *events[FD_SETSIZE];					\
  fd_set readset, writeset;

#endif
