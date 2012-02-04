#define EV_STANDALONE 1
#include "ev.c"

ev_timer* create_timer(void(*cb)(EV_P_ struct ev_timer* w, int revents))
{
  struct ev_loop *loop = EV_DEFAULT;

  ev_timer* timer = calloc(1, sizeof(ev_timer));
  ev_init(timer, cb);
  timer->repeat = 1.0;

  return timer;
}

void delete_timer(ev_timer* timer)
{
  struct ev_loop *loop = EV_DEFAULT;

  ev_timer_stop(loop, timer);
  free(timer);
}

void start_timer(ev_timer* timer)
{
  struct ev_loop *loop = EV_DEFAULT;
  
  ev_timer_again(loop, timer);
}

void stop_timer(ev_timer* timer)
{
  struct ev_loop *loop = EV_DEFAULT;
  
  ev_timer_stop(loop, timer);
}

ev_io* create_io(void(*cb)(EV_P_ struct ev_io* w, int revents), int fd)
{
  struct ev_loop *loop = EV_DEFAULT;

  ev_io* io = calloc(1, sizeof(ev_io));
  ev_init(io, cb);
  ev_io_set(io, fd, EV_READ);

  return io;
}

void delete_io(ev_io* io)
{
  struct ev_loop *loop = EV_DEFAULT;

  ev_io_stop(loop, io);
  free(io);
}

void start_io(ev_io* io)
{
  struct ev_loop *loop = EV_DEFAULT;
  
  ev_io_start(loop, io);
}

void stop_io(ev_io* io)
{
  struct ev_loop *loop = EV_DEFAULT;
  
  ev_io_stop(loop, io);
}

void run(int flags)
{
  struct ev_loop *loop = EV_DEFAULT;

  ev_run(loop, flags);
}
