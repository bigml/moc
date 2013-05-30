#include "moc.h"

/*
 * Hash function used internally by select_srv(). See RFC 1071.
 */
static uint32_t checksum(const unsigned char *buf, size_t bsize)
{
    uint32_t sum = 0;

    while (bsize > 1)  {
        sum += * (uint16_t *) buf++;
        bsize -= 2;
    }

    if (bsize > 0)
        sum += * (uint8_t *) buf;

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

moc_arg* mocarg_init()
{
    moc_arg *arg = calloc(1, sizeof(moc_arg));
    if (!arg) return NULL;

    hash_init(&arg->evth, hash_str_hash, hash_str_comp);
    hash_init(&arg->cbkh, hash_str_hash, hash_str_comp);

#ifdef EVENTLOOP
    mssync_create(&(arg->mainsync));
    mssync_create(&(arg->eloopsync));
    mssync_create(&(arg->callbacksync));
    arg->callbackqueue = msqueue_create();
#endif

    return arg;
}

void mocarg_destroy(moc_arg *arg)
{
    if (!arg) return;

    if (arg->evth) hash_destroy(&arg->evth);
    if (arg->cbkh) hash_destroy(&arg->cbkh);

#ifdef EVENTLOOP
    mssync_destroy(&(arg->mainsync));
    mssync_destroy(&(arg->eloopsync));
    mssync_destroy(&(arg->callbacksync));
    msqueue_destroy(arg->callbackqueue);
    arg->callbackqueue = NULL;
#endif

    free(arg);
}

/*
 * Like recv(), but either fails, or returns a complete read; if we return
 * less than count is because EOF was reached.
 */
ssize_t srecv(int fd, unsigned char *buf, size_t count, int flags)
{
    ssize_t rv, c;

    c = 0;

    while (c < count) {
        rv = recv(fd, buf + c, count - c, flags);

        if (rv == count)
            return count;
        else if (rv < 0)
            return rv;
        else if (rv == 0)
            return c;

        c += rv;
    }
    return count;
}

/*
 * Like srecv(), but for send().
 */
ssize_t ssend(int fd, const unsigned char *buf, size_t count, int flags)
{
    ssize_t rv, c;

    c = 0;

    while (c < count) {
        rv = send(fd, buf + c, count - c, flags);

        if (rv == count)
            return count;
        else if (rv < 0)
            return rv;
        else if (rv == 0)
            return c;

        c += rv;
    }
    return count;
}

/*
 * Compares two servers by their connection identifiers. It is used internally
 * to keep the server array sorted with qsort().
 */
int compare_servers(const void *s1, const void *s2)
{
    moc_srv *srv1 = (moc_srv*) s1;
    moc_srv *srv2 = (moc_srv*) s2;

    in_addr_t a1, a2;
    a1 = srv1->srvsa.sin_addr.s_addr;
    a2 = srv2->srvsa.sin_addr.s_addr;

    if (a1 < a2) {
        return -1;
    } else if (a1 == a2) {
        uint16_t p1, p2;
        p1 = srv1->srvsa.sin_port;
        p2 = srv2->srvsa.sin_port;

        if (p1 < p2)
            return -1;
        else if (p1 == p2)
            return 0;
        else
            return 1;
    } else {
        return 1;
    }
}

/* Selects which server to use for the given key. */
moc_srv *select_srv(moc_t *evt, const char *key, size_t ksize)
{
    uint32_t n;
    
    if (evt->nservers == 0)
        return NULL;
    
    n = checksum((const unsigned char*)key, ksize) % evt->nservers;
    return &(evt->servers[n]);
}

void mutil_utc_time(struct timespec *ts)
{
#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_REALTIME, ts);
#endif    
}
