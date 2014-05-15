/* Win32 Threads with WaitForMultipleObjects */

struct win32thr_arg {
  struct event_queue *evq;
  HANDLE signal;
};

static int
win32thr_sleep (struct win32thr *wth)
{
  const struct event_queue *evq = wth->evq;
  CRITICAL_SECTION *wth_cs;
  int polling, is_ready;

  if (wth == &evq->head)
    return 0;
  wth_cs = &wth->sig_cs;
  polling = 0;

  EnterCriticalSection(wth_cs);
  is_ready = (wth->state == WTHR_READY);
  if (wth->state == WTHR_POLL) {
    wth->state = WTHR_ACK;
    SetEvent(wth->signal);
    polling = 1;
  }
  LeaveCriticalSection(wth_cs);

  if (polling)
    WaitForSingleObject(evq->ack_event, INFINITE);

  return is_ready;
}

static int
win32thr_poll (struct event_queue *evq)
{
  struct win32thr *wth = evq->head.next;
  int changed = 0, polled = 0;
  int oldn = 0;  /* previous available wth->n */

  evq->nwakeup = 0;

  EnterCriticalSection(&evq->head.sig_cs);
  while (wth) {
    CRITICAL_SECTION *wth_cs = &wth->sig_cs;
    int sleeping;

    EnterCriticalSection(wth_cs);
    sleeping = (wth->state == WTHR_SLEEP);
    LeaveCriticalSection(wth_cs);

    if (sleeping) {
      HANDLE signal = wth->signal;

      /* terminate empty thread */
      if (!wth->n) {
        struct win32thr *p;

        /* delayed deletion of last empty thread */
        if (oldn > NEVENT/2 && !wth->next)
          break;

        for (p = &evq->head; p->next != wth; p = p->next)
          continue;

        p->next = wth->next;
        wth = p;
        changed = 1;
      } else {
        oldn = wth->n;
        wth->state = WTHR_POLL;
        evq->nwakeup++;
        polled = 1;
      }
      SetEvent(signal);
    }
    wth = wth->next;
  }
  LeaveCriticalSection(&evq->head.sig_cs);

  /* wait of starting of all polling threads */
  if (polled)
    WaitForSingleObject(evq->ack_event, INFINITE);

  return changed;
}

static DWORD WINAPI
win32thr_wait (struct event_queue *evq)
{
  CRITICAL_SECTION *head_cs;
  HANDLE head_signal;
  struct win32thr wth;

  InitCriticalSection(&wth.sig_cs);
  wth.state = WTHR_SLEEP;
  wth.n = 0;
  wth.tq = NULL;
  wth.handles[0] = wth.signal = ((struct win32thr_arg *) evq)->signal;
  wth.evq = evq = ((struct win32thr_arg *) evq)->evq;
  wth.next = evq->head.next;
  evq->head.next = &wth;
  head_cs = &evq->head.sig_cs;
  head_signal = evq->head.signal;

  SetEvent(evq->ack_event);

  for (; ; ) {
    msec_t now;
    unsigned int res, n;

    WaitForSingleObject(wth.signal, INFINITE);
    if (!(n = wth.n)) break;

    EnterCriticalSection(head_cs);
    now = evq->now;
    if (--evq->nwakeup == 0)
      SetEvent(evq->ack_event);  /* wake up poller */
    LeaveCriticalSection(head_cs);

    res = WaitForMultipleObjects(n + 1, wth.handles, FALSE,
     timeout_get(wth.tq, TIMEOUT_INFINITE, now));
    wth.idx = res;
    res = (res == WAIT_TIMEOUT) || (res < (WAIT_OBJECT_0 + n));

    EnterCriticalSection(&wth.sig_cs);
    if (wth.state == WTHR_ACK) {
      SetEvent(evq->ack_event);
      ResetEvent(wth.signal);
    }
    wth.state = res ? WTHR_READY : WTHR_SLEEP;
    LeaveCriticalSection(&wth.sig_cs);

    if (res) {
      EnterCriticalSection(head_cs);
      wth.next_ready = evq->wth_ready;
      evq->wth_ready = &wth;
      evq->sig_ready |= (1 << EVQ_SIGEVQ);
      SetEvent(head_signal);
      LeaveCriticalSection(head_cs);
    }
  }
  CloseHandle(wth.signal);
  DeleteCriticalSection(&wth.sig_cs);
  return 0;
}

static struct win32thr *
win32thr_init (struct event_queue *evq)
{
  struct win32thr_arg arg;
  HANDLE hThr;
  DWORD id;

  arg.evq = evq;
  arg.signal = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (arg.signal == NULL)
    return NULL;

  hThr = CreateThread(NULL, 4096,
   (LPTHREAD_START_ROUTINE) win32thr_wait, &arg, 0, &id);
  if (hThr == NULL) {
    CloseHandle(arg.signal);
    return NULL;
  }
  SetThreadPriority(hThr, THREAD_PRIORITY_ABOVE_NORMAL);
  CloseHandle(hThr);

  WaitForSingleObject(evq->ack_event, INFINITE);
  return evq->head.next;
}

static int
win32thr_add (struct win32thr *wth, struct event *ev)
{
  struct event_queue *evq = wth->evq;
  HANDLE hEvent;
  unsigned int i = wth->n;

  ev->wth = wth;

  if (!(ev->flags & EVENT_SOCKET))
    hEvent = ev->fd;
  else {
    const unsigned int ev_flags = ev->flags;
    unsigned int event;

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  /* manual-reset */
    if (hEvent == NULL)
      return -1;

    event = (ev_flags & EVENT_READ)
     ? ((ev_flags & EVENT_SOCKET_ACC_CONN) ? FD_ACCEPT : WFD_READ)
     : ((ev_flags & EVENT_SOCKET_ACC_CONN) ? FD_CONNECT : WFD_WRITE);

    if (WSAEventSelect((int) ev->fd, hEvent, event) == SOCKET_ERROR) {
      CloseHandle(hEvent);
      return -1;
    }
  }

  wth->handles[i + 1] = wth->signal;  /* raise signal event */
  wth->handles[i] = hEvent;
  wth->events[i] = ev;
  ev->w.index = i;

  wth->n++;
  evq->nevents++;
  return 0;
}

static int
win32thr_del (struct win32thr *wth, struct event *ev)
{
  int i, n = --wth->n;
  int res = 0;

  if (ev->tq) timeout_del(ev);

  ev->wth = NULL;
  wth->evq->nevents--;

  i = ev->w.index;
  if (ev->flags & EVENT_SOCKET) {
    HANDLE hEvent = wth->handles[i];
    WSAEventSelect((int) ev->fd, hEvent, 0);
    CloseHandle(hEvent);
  } else if (ev->flags & EVENT_DIRWATCH) {
    res = !FindCloseChangeNotification(ev->fd);
  } else if (ev->flags & EVENT_REGWATCH) {
    CloseHandle(ev->fd);
  }

  if (i < n) {
    ev = wth->events[n];
    ev->w.index = i;
    wth->events[i] = ev;
    wth->handles[i] = wth->handles[n];
  }
  wth->handles[n] = wth->signal;  /* lower signal event */

  return res;
}

