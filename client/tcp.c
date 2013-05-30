#include "moc.h"

static int add_tcp_server_addr(moc_t *evt, in_addr_t *inetaddr, int port,
                               int nblock, struct timeval *tv)
{
    int rv, fd;
    moc_srv *newsrv, *newarray;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;

    if (nblock) {
        int x = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, x | O_NONBLOCK);
    } else {
        if (tv->tv_sec != 0 || tv->tv_usec != 0) {
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)tv, sizeof(*tv));
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)tv, sizeof(*tv));
        }
    }
    
    newarray = realloc(evt->servers, sizeof(moc_srv) * (evt->nservers + 1));
    if (newarray == NULL) {
        close(fd);
        return 0;
    }

    evt->servers = newarray;
    evt->nservers++;

    newsrv = &(evt->servers[evt->nservers - 1]);

    newsrv->fd = fd;
    newsrv->srvsa.sin_family = AF_INET;
    newsrv->srvsa.sin_port = htons(port);
    newsrv->srvsa.sin_addr.s_addr = *inetaddr;
    newsrv->nblock = nblock;
    newsrv->tv.tv_sec = tv->tv_sec;
    newsrv->tv.tv_usec = tv->tv_usec;
    newsrv->evt = evt;

    newsrv->buf = NULL;
    newsrv->len = 0;
    newsrv->pktsize = 0;
    newsrv->excess = 0;

    rv = connect(fd, (struct sockaddr *) &(newsrv->srvsa), sizeof(newsrv->srvsa));
    if (rv < 0) goto error_exit;

    /*
     * Disable Nagle algorithm because we often send small packets.
     * Huge gain in performance.
     */
    rv = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &rv, sizeof(rv)) < 0 ) goto error_exit;

    /*
     * keep the list sorted by port, so we can do a reliable selection
     */
    qsort(evt->servers, evt->nservers, sizeof(moc_srv), compare_servers);

    return 1;

error_exit:
    close(fd);
    newarray = realloc(evt->servers, sizeof(moc_srv) * (evt->nservers - 1));
    if (newarray == NULL) {
        evt->servers = NULL;
        evt->nservers = 0;
        return 0;
    }

    evt->servers = newarray;
    evt->nservers -= 1;

    return 0;
}

static int tcp_server_reconnect(moc_srv *srv)
{
    int rv, fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;

    if (srv->nblock) {
        int x = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, x | O_NONBLOCK);
    } else {
        if (srv->tv.tv_sec != 0 || srv->tv.tv_usec != 0) {
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&(srv->tv), sizeof(srv->tv));
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&(srv->tv), sizeof(srv->tv));
        }
    }
    
    rv = connect(fd, (struct sockaddr *) &(srv->srvsa), sizeof(srv->srvsa));
    if (rv < 0) goto error_exit;

    /*
     * Disable Nagle algorithm because we often send small packets.
     * Huge gain in performance.
     */
    rv = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &rv, sizeof(rv)) < 0 ) goto error_exit;

    srv->fd = fd;

    if (srv->buf) free(srv->buf);
    srv->buf = NULL;
    srv->len = 0;
    srv->pktsize = 0;
    srv->excess = 0;

    return 1;

error_exit:
    close(fd);

    return 0;
}

static ssize_t recv_msg(int fd, unsigned char *buf, size_t bsize)
{
    ssize_t rv, t;
    uint32_t msgsize;

    rv = recv(fd, buf, bsize, 0);
    if (rv <= 0) return rv;

    if (rv < 4) {
        t = srecv(fd, buf + rv, 4 - rv, 0);
        if (t <= 0) {
            return t;
        }

        rv = rv + t;
    }

    msgsize = * ((uint32_t *) buf);
    msgsize = ntohl(msgsize);

    if (msgsize > bsize)
        return -1;

    if (rv < msgsize) {
        t = srecv(fd, buf + rv, msgsize - rv, 0);
        if (t <= 0) {
            return t;
        }

        rv = rv + t;
    }

    return rv;
}

int moc_add_tcp_server(moc_t *evt, const char *addr, int port,
                       int nblock, void *tv)
{
    int rv;
    struct hostent *he;
    struct in_addr ia;

    /*
     * We try to resolve and then pass it to add_tcp_server_addr().
     */
    rv = inet_pton(AF_INET, addr, &ia);
    if (rv <= 0) {
        he = gethostbyname(addr);
        if (he == NULL)
            return 0;

        ia.s_addr = *( (in_addr_t *) (he->h_addr_list[0]) );
    }

    return add_tcp_server_addr(evt, &(ia.s_addr), port, nblock, (struct timeval*)tv);
}

int tcp_srv_send(moc_srv *srv, unsigned char *buf, size_t bsize)
{
    ssize_t rv;
    uint32_t len;

    len = htonl(bsize);
    memcpy(buf, (const void *) &len, 4);

    if (srv->fd <= 0) {
        mtc_dbg("connection closed, reconnect");
        /*
         * TODO
         * although we reconnect to the server agin,
         * but we haven't do application's JOIN command to do further communicate
         * so, application need do JOIN on reconnect
         * need a system_callback here
         */
        if (!tcp_server_reconnect(srv)) {
            mtc_dbg("reconnect failure");
            return 0;
        }
    }

    rv = ssend(srv->fd, buf, bsize, 0);
    if (rv != bsize) return 0;
    return 1;
}

/*
 * get and parse replies from the server.
 */
uint32_t tcp_get_rep(moc_srv *srv, unsigned char *buf, size_t bsize,
                     unsigned char **payload, size_t *psize)
{
    ssize_t rv;
    uint32_t id, reply;

rerecv:
    rv = recv_msg(srv->fd, buf, bsize);
    if (rv <= 0) return -1;

    id = * ((uint32_t *) buf + 1);
    id = ntohl(id);
    reply = * ((uint32_t *) buf + 2);
    reply = ntohl(reply);

    if (id < g_reqid) goto rerecv;

    if (payload != NULL) {
        *payload = buf + 4 + 4 + 4;
        *psize = rv - 4 - 4 - 4;
    }
    
    return reply;
}
