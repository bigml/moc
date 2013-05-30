#include "mheads.h"
#include "lheads.h"

static void time_up(int fd, short flags, void* arg)
{
    struct event *ev = (struct event*)arg;
    struct timeval t = {.tv_sec = 1, .tv_usec = 0};
    static bool initialized = false;
    static int intime = 0;
    intime++;

    if (initialized) event_del(ev);
    else initialized = true;

    evtimer_set(ev, time_up, ev);
    evtimer_add(ev, &t);

    g_ctime = time(NULL);

    struct event_chain *c;
    struct event_entry *e;
    for (size_t i = 0; i < g_moc->hashlen; i++) {
        c = g_moc->table + i;

        e = c->first;
        while (e) {
            struct timer_entry *t = e->timers;
            while (t && t->timeout > 0) {
                if (intime % t->timeout == 0) {
                    t->timer(e, intime);
                    if (!t->repeat) t->timeout = 0;
                }
                t = t->next;
            }
            e = e->next;
        }
    }
}

void net_go()
{
    struct event ev, ev_clock;
    int fd = -1;
    char *ip;
    int port;

    ip = hdf_get_value(g_cfg, PRE_SERVER".ip", "127.0.0.1");
    port = hdf_get_int_value(g_cfg, PRE_SERVER".port", 5000);

    /*
     * we use DEPRECATED event_init() here because:
     * event_init() more simple than event_base_xxx(),
     * and I can make sure the server's event just used by the main thread only
     */
    event_init();

    fd = tcp_init(ip, port);
    if (fd <= 0) {
        mtc_err("init tcp socket on %s %d failure %d", ip, port, fd);
        return;
    }
    
    event_set(&ev, fd, EV_READ | EV_PERSIST, tcp_newconnection, &ev);
    event_add(&ev, NULL);

    struct timeval t = {.tv_sec = 1, .tv_usec = 0};
    evtimer_set(&ev_clock, time_up, &ev_clock);
    evtimer_add(&ev_clock, &t);

    event_dispatch();

    event_del(&ev);
    event_del(&ev_clock);
    
    tcp_close(fd);
}
