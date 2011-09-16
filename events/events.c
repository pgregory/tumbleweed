#define EV_STANDALONE 1
#include "ev.c"

static void timer_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
  printf("Tick!\n");
  ev_timer_again(loop, w);
}

void ev_test()
{
  struct ev_loop *loop = EV_DEFAULT;

  ev_timer timer_hit;
  ev_init(&timer_hit, timer_cb);
  timer_hit.repeat = 1.0;
  ev_timer_again(loop, &timer_hit);

  ev_run(loop, 0);
}
