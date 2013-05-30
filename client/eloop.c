#include "moc.h"

struct el_con {
    char *name;
    int order;                  /* used on reconnect */
    int fd;
};

static pthread_t *m_thread = NULL;
static bool m_stop = false;
static unsigned char static_buf[SBSIZE];

static void* el_routine(void *arg)
{
    moc_arg *earg = (moc_arg*)arg;
    HASH *evth = (HASH*)earg->evth;
    char *key = NULL;

    struct el_con conn[MOC_MAX_CON];
    int num_conn = 0;
    
    mtc_dbg("start event loop thread...");

    fd_set readset;
    struct timeval tv;
    for (;;) {
        memset(conn, sizeof(conn), 0x0);
        num_conn = 0;
        key = NULL;

        /*
         * we need refresh conn[] array after server_reconnect()
         * we can do this by signal, but it's complex
         * and the num_conn normaly <= 1000
         * so, refresh conn[] per select(or per 10 select :D).
         */
        moc_t *evt = hash_next(evth, (void**)&key);
        while (evt) {
            for (int i = 0; i < evt->nservers && num_conn < MOC_MAX_CON; i++) {
                moc_srv *srv = &(evt->servers[i]);
                if (srv->fd > 0) {
                    conn[num_conn].name = strdup(evt->ename);
                    conn[num_conn].order = i;
                    conn[num_conn].fd = srv->fd;
                
                    num_conn++;
                }
            }
            
            evt = hash_next(evth, (void**)&key);
        }
        
        int maxfd = -1;
        FD_ZERO(&readset);

        for (int i = 0; i < num_conn; i++) {
            if (conn[i].fd > 0) {
                if (conn[i].fd > maxfd) maxfd = conn[i].fd;
                FD_SET(conn[i].fd, &readset);
            }
        }

        tv.tv_sec = 2;
        tv.tv_usec = 0;
        select(maxfd + 1, &readset, NULL, NULL, &tv);

        for (int i = 0; i < num_conn; i++) {
            if (FD_ISSET(conn[i].fd, &readset)) {
                int rv = recv(conn[i].fd, static_buf, SBSIZE, 0);

                mtc_dbg("msg from %s %d fd %d len %d",
                        conn[i].name, conn[i].order, conn[i].fd, rv);

                if (rv < 0 && errno == EAGAIN) {
                    /*
                     * We were awoken but have no data to read, so we do nothing
                     */
                    continue;
                } else if (rv <= 0) {
                    /*
                     * Orderly shutdown or error;
                     * close the file descriptor in either case.
                     */
                    evt = hash_lookup(evth, conn[i].name);
                    if (evt) mssrv_close(evt, conn[i].order, conn[i].fd);

                    conn[i].fd = -1;
                }

                evt = hash_lookup(evth, conn[i].name);
                if (!evt) mtc_err("%s not in evth", conn[i].name);
                else {
                    moc_srv *srv = &(evt->servers[conn[i].order]);
                    if (!srv) {
                        mtc_err("%s out of order %d %d",
                                conn[i].name, conn[i].order, evt->nservers);
                        continue;
                    }
                    if (srv->buf == NULL)
                        msparse_buf(evt, conn[i].order, conn[i].fd,
                                    static_buf, rv, earg);
                    else {
                        memcpy(srv->buf + srv->len, static_buf, rv);
                        srv->len += rv;
                        msparse_buf(evt, conn[i].order, conn[i].fd,
                                    srv->buf, srv->len, earg);
                    }
                }
            }
        }

        if (m_stop) break;
    }

    return NULL;
}

NEOERR* eloop_start(moc_arg *arg)
{
    if (m_thread) return nerr_raise(NERR_ASSERT, "eloop started already");

    m_stop = false;
    m_thread = calloc(1, sizeof(pthread_t));
    pthread_create(m_thread, NULL, el_routine, (void*)arg);

    return STATUS_OK;
}

void eloop_stop(moc_arg *arg)
{
    if (!m_thread) return;
    
    mtc_dbg("end event loop thread...");

    m_stop = true;
    pthread_join(*m_thread, NULL);
    free(m_thread);
    m_thread = NULL;
}
