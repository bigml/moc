#include "moc.h"

static pthread_t *m_thread = NULL;
static bool m_stop = false;

static void* callback_routine(void *arg)
{
    moc_arg *earg = (moc_arg*)arg;
    HASH *cbkh = (HASH*)earg->cbkh;
    struct timespec ts;
    struct msqueue_entry *e;
    int rv;

    mtc_dbg("start callback thread...");

    for (;;) {
        mutil_utc_time(&ts);
        ts.tv_sec += 1;
        rv = 0;

        mssync_lock(&earg->callbacksync);
        while (msqueue_isempty(earg->callbackqueue) && rv == 0) {
            rv = mssync_timedwait(&earg->callbacksync, &ts);
        }
        
        if (rv != 0 && rv != ETIMEDOUT) {
            mtc_err("Error in timedwait() %d", rv);
            continue;
        }

        e = msqueue_get(earg->callbackqueue);
        mssync_unlock(&earg->callbacksync);

        if (!e) {
            if (m_stop) break;
            else continue;
        }

        /*
         * call callback on e
         */
        struct moc_cbk *c = mcbk_find(cbkh, e->ename, e->cmd);
        if(c) c->callback(e->hdfrcv);

        msqueue_entry_destroy(e);
    }

    return NULL;
}

NEOERR* mcbk_start(moc_arg *arg)
{
    if (m_thread) return nerr_raise(NERR_ASSERT, "mcbk started already");

    m_stop = false;
    m_thread = calloc(1, sizeof(pthread_t));
    pthread_create(m_thread, NULL, callback_routine, (void*)arg);

    return STATUS_OK;
}

void mcbk_stop(moc_arg *arg)
{
    if (!m_thread) return;
    
    mtc_dbg("end callback thread...");

    m_stop = true;
    pthread_join(*m_thread, NULL);
    free(m_thread);
    m_thread = NULL;
}

struct moc_cbk* mcbk_create()
{
    struct moc_cbk *c = calloc(1, sizeof(struct moc_cbk));

    return c;
}

void mcbk_destroy(struct moc_cbk *c)
{
    if (!c) return;
    if (c->ename) free(c->ename);
    if (c->cmd) free(c->cmd);
    free(c);
}

struct moc_cbk* mcbk_find(HASH *cbkh, char *module, char *cmd)
{
    char tok[1024];

    memset(tok, sizeof(tok), 0x0);
    snprintf(tok, sizeof(tok), "%s_%s", module, cmd);

    mtc_dbg("lookup %s in cbkh", tok);

    return (struct moc_cbk*)hash_lookup(cbkh, tok);
}

void mcbk_regist(HASH *cbkh, char *module, char *cmd, struct moc_cbk *c)
{
    char tok[1024];

    memset(tok, sizeof(tok), 0x0);
    snprintf(tok, sizeof(tok), "%s_%s", module, cmd);

    mtc_dbg("regist %s in cbkh", tok);

    hash_insert(cbkh, (void*)strdup(tok), (void*)c);
}
