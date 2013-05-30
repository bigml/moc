#include "moc.h"

/*
 * sync
 */
void mssync_create(struct mssync *s)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    pthread_mutex_init(&s->lock, &attr);
    pthread_mutexattr_destroy(&attr);

    pthread_cond_init(&s->cond, NULL);
}

void mssync_destroy(struct mssync *s)
{
    pthread_mutex_destroy(&(s->lock));
}

void mssync_lock(struct mssync *s)
{
    pthread_mutex_lock(&(s->lock));
}

void mssync_unlock(struct mssync *s)
{
    pthread_mutex_unlock(&(s->lock));
}

void mssync_signal(struct mssync *s)
{
    pthread_cond_signal(&(s->cond));
}

int mssync_timedwait(struct mssync *s, struct timespec *ts)
{
    return pthread_cond_timedwait(&(s->cond), &(s->lock), ts);
}


/*
 * queue
 */
struct msqueue* msqueue_create()
{
    struct msqueue *q;

    q = calloc(1, sizeof(struct msqueue));
    if (!q) {
        mtc_err("alloc msqueue");
        return NULL;
    }

    q->size = 0;
    q->top = NULL;
    q->bottom = NULL;

    return q;
}

void msqueue_destroy(struct msqueue *q)
{
    if (!q) return;

    struct msqueue_entry *e;

    e = msqueue_get(q);
    while (e) {
        msqueue_entry_destroy(e);
        e = msqueue_get(q);
    }

    free(q);
    return;
}

int msqueue_isempty(struct msqueue *q)
{
    return (q->size == 0);
}

struct msqueue_entry* msqueue_entry_create()
{
    struct msqueue_entry *e = calloc(1, sizeof(struct msqueue_entry));
    if (!e) {
        mtc_err("out of memory");
        return NULL;
    }

    e->ename = NULL;
    e->cmd = NULL;

    hdf_init(&e->hdfrcv);
    hdf_init(&e->hdfsnd);
    
    e->prev = NULL;

    return e;
}

void msqueue_entry_destroy(struct msqueue_entry *e)
{
    if (!e) return;
    if (e->ename) free(e->ename);
    if (e->cmd) free(e->cmd);
    hdf_destroy(&e->hdfrcv);
    hdf_destroy(&e->hdfsnd);
    free(e);

    return;
}

struct msqueue_entry *msqueue_get(struct msqueue *q)
{
    struct msqueue_entry *e, *t;

    if (q->bottom == NULL)
        return NULL;

    e = q->bottom;
    t = q->bottom->prev;
    q->bottom = t;
    if (t == NULL) {
        /* it's empty now */
        q->top = NULL;
    }
    q->size -= 1;
    return e;
}

void msqueue_put(struct msqueue *q, struct msqueue_entry *e)
{
    if (q->top == NULL) {
        q->top = q->bottom = e;
    } else {
        q->top->prev = e;
        q->top = e;
    }
    q->size += 1;
    return;
}

void msqueue_cas(struct msqueue *q, struct msqueue_entry *e)
{
    if (q->top == NULL) {
        q->top = q->bottom = e;
    } else {
        e->prev = q->bottom;
        q->bottom = e;
    }
    q->size += 1;
    return;
}


/*
 * application logic message, called by msparse_buf()
 */
static NEOERR* msparse_msg(moc_srv *srv, unsigned char *buf, size_t len, moc_arg *arg)
{
    uint32_t id, reply;
    unsigned char *payload;
    size_t psize, rv;

    MOC_NOT_NULLB(arg, arg->evth);
    
    if (!srv || !buf || len < 8) return nerr_raise(NERR_ASSERT, "illegal packet");

    //MSG_DUMP("recv: ", buf, len);

    /* The header is:
     * 4 bytes    ID
     * 4 bytes    Reply Code
     * Variable   Payload
     */
    id = ntohl(* ((uint32_t *) buf));
    reply = ntohl(* ((uint32_t *) buf + 1));

    payload = buf + 8;
    psize = len - 8;

    if (id == 0 && reply == 10000) {
        /*
         * server push
         */
        if (psize < 4) return nerr_raise(NERR_ASSERT, "server pushed empty message");

        struct msqueue_entry *e = msqueue_entry_create();
        if (!e) return nerr_raise(NERR_NOMEM, "alloc msqueue entry");
        
        rv = unpack_hdf(payload + 4, psize - 4, &(e->hdfrcv));
        if (rv <= 0) return nerr_raise(NERR_ASSERT, "server pushed illegal message");

        //TRACE_HDF(e->hdfrcv);

        char *cmd = NULL;
        HDF_ATTR *attr = hdf_get_attr(e->hdfrcv, "_Reserve");
        while (attr != NULL) {
            if (!strcmp(attr->key, "cmd")) cmd = attr->value;
            attr = attr->next;
        }
        if (!cmd) return nerr_raise(NERR_ASSERT, "cmd not supplied");

        e->ename = strdup(srv->evt->ename);
        e->cmd = strdup(cmd);

        mtc_dbg("receive cmd %s", cmd);
        
        hdf_remove_tree(e->hdfrcv, "_Reserve");

        mssync_lock(&arg->callbacksync);
        msqueue_put(arg->callbackqueue, e);
        mssync_unlock(&arg->callbacksync);

        /*
         * notify callback thread
         */
        mssync_signal(&arg->callbacksync);
    } else {
        /*
         * server response
         */
        if (id < g_reqid)
            return nerr_raise(NERR_ASSERT, "id not match %d %d", g_reqid, id);

        if (psize >= 4) {
            mssync_lock(&(arg->mainsync));
            rv = unpack_hdf(payload + 4, psize - 4, &(srv->evt->hdfrcv));
            mssync_unlock(&(arg->mainsync));
            if (rv <= 0)
                return nerr_raise(NERR_ASSERT, "server responsed illegal message");

            TRACE_HDF(srv->evt->hdfrcv);
        }

        /*
         * notify main thread
         */
        mssync_signal(&(arg->mainsync));
    }
    
    return STATUS_OK;
}

void msparse_buf(moc_t *evt, int order, int fd,
                 unsigned char *buf, size_t len, moc_arg *arg)
{
    uint32_t totaltoget = 0;
    moc_srv *srv;
    NEOERR *err;

    if (!evt || !buf) {
        mtc_err("param null");
        return;
    }

    /*
     * validate
     */
    if (evt->nservers < order) {
        mtc_err("order %d > server %d", order, evt->nservers);
        return;
    }
    
    srv = &(evt->servers[order]);
    if (srv->fd != fd) {
        mtc_err("fd not equal %d %d", srv->fd, fd);
        return;
    }
    
    /*
     * copy from process_buf() on server/tcp.c
     */
    if (len >= 4) {
        totaltoget = * (uint32_t*) buf;
        totaltoget = ntohl(totaltoget);
        if (totaltoget > (64 * 1024) || totaltoget <= 8) {
            mtc_err("message illegal %d", totaltoget);
            return;
        }
    } else totaltoget = 4;

    if (totaltoget > len) {
        if (srv->buf == NULL) {
            srv->buf = malloc(SBSIZE);
            if (!srv->buf) {
                mtc_foo("out of memory");
                return;
            }

            memcpy(srv->buf, buf, len);
            srv->len = len;
        } else {
            srv->len = len;
        }

        srv->pktsize = totaltoget;

        /* need rereceive */
        return;
    }

    if (totaltoget < len) {
        srv->excess = len - totaltoget;
        len = totaltoget;
    }

    err = msparse_msg(srv, buf + 4, len - 4, arg);
    TRACE_NOK(err);

    if (srv->excess) {
        memmove(buf, buf + len, srv->excess);
        srv->len = srv->excess;
        srv->excess = 0;

        msparse_buf(evt, order, fd, buf, srv->len, arg);
        /* need reprocess */
        return;
    }

    if (srv->buf) {
        free(srv->buf);
        srv->buf = NULL;
        srv->len = 0;
        srv->pktsize = 0;
        srv->excess = 0;
    }

    return;
}

void mssrv_close(moc_t *evt, int order, int fd)
{
    if (!evt) return;
    
    mtc_dbg("%s %d %d closed", evt->ename, order, fd);

    if (evt->nservers < order) return;

    moc_srv *srv = &(evt->servers[order]);

    if (srv->fd != fd) return;

    close(srv->fd);
    srv->fd = -1;
}
